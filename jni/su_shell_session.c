/*
 * libsusrv: Android SU native client library.
 *
 * <t0kt0ckus@gmail.com>
 * (C) 2014
 *
 * License: GPLv3
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#include "su_srv_log.h"
#include "su_shell_session.h"

///////////////////////////////////////////////////////////////////////////////////////////////
//
// Constants
//

static const char *SESSION_AFUN_FMT = "%s/var/run/su_session-%05d"; 
static const int SESSION_AFUN_FMT_WLEN = 20 + 5;
static const int BUF_INITIAL_SIZE = 32;

///////////////////////////////////////////////////////////////////////////////////////////////
//
// FORWARDs definitions
//

static int init_mutexes(su_shell_session *session);
static void destroy_mutexes(su_shell_session *session);
static int init_af_un_address(su_shell_session *session);

///////////////////////////////////////////////////////////////////////////////////////////////
//
// API: su_session.h
//
///////////////////////////////////////////////////////////////////////////////////////////////

su_shell_session *su_shell_session_new(const char * pfs_root, pid_t owner_pid)
{
    su_shell_session *session = malloc(sizeof(su_shell_session));
    if (session)
    {
        session->owner_pid = owner_pid;
        session->pfs_root = pfs_root;
        session->afun_client_fd = 0;
        session->shell_pid = (pid_t) 0;
        session->handler_pth = malloc(sizeof(pthread_t));
        session->handler_buf = malloc(sizeof(char) * BUF_INITIAL_SIZE);

        if (init_af_un_address(session) || init_mutexes(session))
        {
            if (session->handler_pth)
                free(session->handler_pth);
            if (session->handler_buf)
                free(session->handler_buf);
            free(session);
            session = NULL;
        }
    }

    return session;
}

int su_shell_session_af_un_rdv(su_shell_session *session)
{
    int un_peer_fd = -1;

    int un_rdv_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (un_rdv_fd)
    {
        if ((bind(un_rdv_fd,
                 (struct sockaddr *) &session->afun_rdv_addr,
                 sizeof(struct sockaddr_un)) == 0)
            && (listen(un_rdv_fd, 1) == 0)
            && ((session->afun_client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) > 0))
        {
            socklen_t un_socklen = sizeof(struct sockaddr_un);

            if (connect(session->afun_client_fd,
                        (struct sockaddr *) &session->afun_rdv_addr,
                        sizeof(struct sockaddr_un))
                ||
                ((un_peer_fd = accept(un_rdv_fd,
                                      (struct sockaddr*) &session->afun_rdv_addr,
                                      &un_socklen)) < 0))
            {
                su_srv_log_perror("Failed to establish AF UNIX rendez-vous");
            }
        
            // end-of session->afun_client_fd
        }
        else
        {
            su_srv_log_perror("Failed to setup AF UNIX rendez-vous");    
        }

        close(un_rdv_fd);
    } // end-of un_rdv_fd

    return un_peer_fd;
}

void su_shell_session_delete(su_shell_session *session)
{
    destroy_mutexes(session);

    if (session->afun_client_fd > 0)
    {
        close(session->afun_client_fd);
        unlink(session->afun_rdv_addr.sun_path);
    }

    if (session->handler_pth)
        free(session->handler_pth);

   if (session->handler_buf)
        free(session->handler_buf);

    free(session);
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
// FORWARDs implementation
//

int init_af_un_address(su_shell_session *session)
{
    int afun_path_len = strlen(session->pfs_root) + SESSION_AFUN_FMT_WLEN + 1;
    if (afun_path_len > AF_UNIX_PATH_MAX)
        return -1;

    char * afun_path = malloc(sizeof(char) * afun_path_len);
    if (afun_path)
    {
        session->afun_rdv_addr.sun_family = AF_UNIX;
        
        snprintf(afun_path,
                 afun_path_len,
                 SESSION_AFUN_FMT,
                 session->pfs_root,
                 session->owner_pid);
        strcpy(session->afun_rdv_addr.sun_path, afun_path);

        free(afun_path);
        return 0;
   }

   return -1;   
}

int init_mutexes(su_shell_session *session)
{
    int last_err = 0;
    if ((session->shell_sync_mutex = malloc(sizeof(pthread_mutex_t)))
            && (last_err = pthread_mutex_init(session->shell_sync_mutex, NULL))) 
    {
        free(session->shell_sync_mutex);
        session->shell_sync_mutex = NULL;
    }

    if ((! last_err) && (session->shell_sync_ready = malloc(sizeof(pthread_cond_t)))
            && (last_err = pthread_cond_init(session->shell_sync_ready, NULL)))
    {
        free(session->shell_sync_ready);
        session->shell_sync_ready = 0;
    }
    
    return last_err;
}

void destroy_mutexes(su_shell_session *session)
{
    pthread_mutex_unlock(session->shell_sync_mutex);

    pthread_cond_destroy(session->shell_sync_ready);
    free(session->shell_sync_ready);
    session->shell_sync_ready = NULL;

    pthread_mutex_destroy(session->shell_sync_mutex);
    free(session->shell_sync_mutex);
    session->shell_sync_mutex = NULL;
}

