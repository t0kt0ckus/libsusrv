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
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>

#include "su_shell_session.h"
#include "su_srv_pfs.h"
#include "su_srv_log.h"
#include "su_srv.h"


/////////////////////////////////////////////////////////////////////////////////////////////
//
// Constants
//

#ifdef SU_SRV_TEST_BUILD
static char *SU_SHELL_BINARY_CANDIDATES[] = {
        "/bin/bash", "/sbin/su", "/usr/bin/su", "/usr/sbin/su", NULL
};
static char * const SU_SHELL_ENVIRONMENT[] = { NULL };
#else
static char *SU_SHELL_BINARY_CANDIDATES[] = {
        "/sbin/su", "/system/sbin/su", "/system/bin/su", "/system/xbin/su", NULL
};
static char * const SU_SHELL_ENVIRONMENT[] = {
        "PATH=/sbin:/vendor/bin:/system/sbin:/system/bin:/system/xbin", NULL
};
#endif

static const char * EOC_CMD = "echo \"<SU_SRV_EOC>\"$?\n";
static const char * EOC_TAG = "<SU_SRV_EOC>";
static const char * EOC_SSCAN ="<SU_SRV_EOC>%d\n";

static const int WAIT_CHILD_MAX_RETRY = 3;
static const int WAIT_CHILD_SLEEP_NSEC = 250000000;
static const int WAIT_CHILD_SLEEP_SEC = 1;


/////////////////////////////////////////////////////////////////////////////////////////////
//
// FORWARD definitions
//

static char *find_su_binary();

static int pth_block_sigchld();
static int pth_unblock_sigchld();

static void *session_handler_fn(void *);

static int create_handler_thread(su_shell_session *session, void *(* handler_fn)(void *));

static int start_shell_process(int fd, su_shell_session *session);
static int stop_shell_process(su_shell_session *session);

static int acquire_handler_mutex(su_shell_session *session);
static void send_exit_command(su_shell_session *session);
static void delete_current_session();


///////////////////////////////////////////////////////////////////////////////////////////////
//
//                                         State

static su_shell_session *_su_session;
static pthread_mutex_t _su_session_mutex = PTHREAD_MUTEX_INITIALIZER;
static sigset_t SU_SRV_SIGMASK;

///////////////////////////////////////////////////////////////////////////////////////////////
//
// API: su_session.h
//
///////////////////////////////////////////////////////////////////////////////////////////////

int su_srv_open_shell_session(const char *pfs_root)
{
    int last_err = pthread_mutex_lock(&_su_session_mutex);
    if (last_err)
    {
        su_srv_log_printf("Warning, failed to initiate session startup");
    }
    else
    {
        if (_su_session)
            last_err = SU_SRV_SESSION_ALREADY_OPEN_ERROR;
        else
        {
            if (su_srv_pfs_init(pfs_root))
                last_err = SU_SRV_PFS_ERROR;
            else
            {
                pid_t owner_pid = getpid();
                su_shell_session *session;
   
                su_srv_log_init(pfs_root, owner_pid);

                if ((session = su_shell_session_new(pfs_root, owner_pid)) == NULL)
                    last_err = SU_SRV_SYSTEM_ERROR;
                else
                {
                    su_srv_log_printf("Initializing SU shell session:");
                    su_srv_log_printf("owner PID: %d", session->owner_pid);
                    su_srv_log_printf("AF UNIX path: %s", session->afun_rdv_addr.sun_path);

                    int peer_fd = su_shell_session_af_un_rdv(session);
                    if (peer_fd > 0)
                    {
                        su_srv_log_printf("AF UNIX rendez-vous complete");

                        if ((last_err = start_shell_process(peer_fd, session)))
                            su_srv_log_printf("Failed to create SU shell child process !");
                        else
                        {
                            su_srv_log_printf("Created SU shell child process (PID: %d)",
                                    session->shell_pid);

                            if ((last_err = create_handler_thread(session, session_handler_fn)))
                            {
                                su_srv_log_printf("Failed to create handler loop's thread !");
                                // when unable to start handler loop, stops shell process
                                stop_shell_process(session);
                            }
                        } // end-of shell subprocess valid

                        close(peer_fd);
                    } // end-of un_peer_fd valid
                
                    if (last_err)
                    {
                        su_srv_log_printf("Failed to initialize SU shell session !");
                        su_shell_session_delete(session);
                    }
                    else
                    {
                        su_srv_log_printf("SU shell session initialization complete");
                        _su_session = session;
                    }

                } // end-of session allocated
            } // end-of pfs error
        } // end-of attempt to create session

        pthread_mutex_unlock(&_su_session_mutex);
    } // end-of lock

    return last_err;
}

int su_srv_exec(const char *cmd_str)    
{
    if (! _su_session)
    {
        su_srv_log_printf("Ignored exec(), no session");
        return SU_SRV_NO_SESSION_ERROR;
    }

    if (pthread_mutex_lock(_su_session->shell_sync_mutex) == 0)
    {
        su_srv_log_cmdstr(cmd_str);

        write(_su_session->afun_client_fd, cmd_str, strlen(cmd_str));  
        write(_su_session->afun_client_fd, "\n", 1);
        write(_su_session->afun_client_fd, EOC_CMD, strlen(EOC_CMD));  
        write(_su_session->afun_client_fd, "\n", 1);

        _su_session->shell_cmd_exit_code = SU_SRV_EXIT_CODE_UNKNOWN;
        pthread_cond_wait(_su_session->shell_sync_ready, _su_session->shell_sync_mutex);
        pthread_mutex_unlock(_su_session->shell_sync_mutex);

        return _su_session->shell_cmd_exit_code;
    }
    else
    {
        su_srv_log_printf("Failed to acquire lock on exec()");
        return SU_SRV_SYSTEM_ERROR;
    }
}

int su_srv_exit_shell_session()
{

    int last_err = pthread_mutex_lock(&_su_session_mutex);
    if (last_err)
    {
        su_srv_log_printf("Failed to initiate session shutdown");
    }
    else
    {
        if (! _su_session)
        {
            su_srv_log_printf("Ignored exit(), no session");
            last_err = SU_SRV_NO_SESSION_ERROR;
        }
        else
        {
            su_srv_log_printf("SU shell sesion shutdown ...");

            if ((last_err = stop_shell_process(_su_session)))
                su_srv_log_printf("Failed to stop SU shell, expect a zombie process !");

            if ( (last_err = pthread_join(*_su_session->handler_pth, NULL)) )
                su_srv_log_printf("Failed to join(), expect a zombie thread !");

            delete_current_session();
        }

        pthread_mutex_unlock(&_su_session_mutex);
    }


    su_srv_log_printf("SU shell session closed");
    return last_err;
}

int su_srv_shell_session_stat() 
{
    return (_su_session != NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// FORWARDs implementation
//

int acquire_handler_mutex(su_shell_session *session)
{
    int last_err = pthread_mutex_trylock(session->shell_sync_mutex);
    if (last_err)
    {
        pthread_cond_signal(session->shell_sync_ready);
        last_err = pthread_mutex_lock(session->shell_sync_mutex);
    }

    return last_err;
}


void *session_handler_fn(void * targs)
{
    int fd = fileno(su_srv_log_fptr);
      
    int curr_line_offset = 0;
    char curr_byte;

    while (read(_su_session->afun_client_fd, &curr_byte, 1) > 0) 
    {
        if (curr_line_offset > (sizeof(_su_session->handler_buf) -1 -1) )
        {
            _su_session->handler_buf = realloc(_su_session->handler_buf,
                                           (curr_line_offset + 32) * sizeof(char));
        }
        _su_session->handler_buf[curr_line_offset++] = curr_byte;

        if (curr_byte == 10) // LF
        {
            _su_session->handler_buf[curr_line_offset] = 0;
            curr_line_offset = 0;

            if (strstr(_su_session->handler_buf, EOC_TAG))
            {
                if (sscanf(_su_session->handler_buf,
                            EOC_SSCAN,
                            &_su_session->shell_cmd_exit_code) == EOF)
                {
                    _su_session->shell_cmd_exit_code = SU_SRV_EXIT_CODE_UNKNOWN;
                }

                pthread_cond_signal(_su_session->shell_sync_ready);          
            }
            else
            {
                write(fd, _su_session->handler_buf, strlen(_su_session->handler_buf));
            }      
        }
    }

    // when NOT on exiting upon su_srv_exit_session(), cleanup session here
    if (pthread_mutex_trylock(&_su_session_mutex) != EBUSY)
    {
        su_srv_log_printf("Warning, discarding session upon unexpected shell process exit");
        delete_current_session();

        pthread_mutex_unlock(&_su_session_mutex);
    }
    pthread_exit(NULL);  
}

int start_shell_process(int fd, su_shell_session *session)
{
    char * su_binary = find_su_binary();
    if (su_binary == NULL)
    {
        su_srv_log_printf("Failed to locate any suitable su binary");
        return SU_SRV_NO_SYSTEM_SU_ERROR;
    }
    su_srv_log_printf("Found system SU binary: %s", su_binary);

    pid_t shell_pid = fork();
    if (shell_pid < 0)
    {
        su_srv_log_perror("Failed to fork()");
        return SU_SRV_SYSTEM_ERROR;
    }
    
    if (shell_pid == 0)
    {
            char *const params[2] = {su_binary, NULL};

            dup2(fd,0);
            dup2(0,1);
            dup2(0,2);
            execve(su_binary, params, SU_SHELL_ENVIRONMENT);
    }
    else
    {
        session->shell_pid = shell_pid;
    }

    return 0;
}

int stop_shell_process(su_shell_session *session)
{
    int waited = 0;
    int again = 0;
    int wstatus;

    // send exit command and wait a bit
    struct timespec req_timeout;
    req_timeout.tv_sec = 0;
    req_timeout.tv_nsec = WAIT_CHILD_SLEEP_NSEC;
    send_exit_command(_su_session);
    nanosleep(&req_timeout, NULL);
         
    while ((again < WAIT_CHILD_MAX_RETRY) && (! waited))
    {
        // note: we use waitpid()/sleep() as sigtimedwait() is not defined by Android libc
     
        if ( (waited = waitpid(session->shell_pid, &wstatus, WNOHANG)) == 0)
        {
            if (again++ == 0)
            {
                // try to KILL on 1rst retry
                su_srv_log_printf("EAGAIN: kill(%d, SIGKILL)", session->shell_pid);
                kill(session->shell_pid, SIGKILL);
            }

            sleep(WAIT_CHILD_SLEEP_SEC);
            su_srv_log_printf("EAGAIN: sleep ...");
        }
        else if (waited < 0)
            su_srv_log_perror("Failed to waitpid()");

    } // end-while-not-waited 
    
    return (waited == session->shell_pid ) ? 0 : -1;
}

int create_handler_thread(su_shell_session *session, void *(* handler_fn)(void *))
{
    pth_block_sigchld();
    int pth = pthread_create(session->handler_pth, 0, handler_fn, NULL);
    pth_unblock_sigchld();

    return pth;
}

int pth_block_sigchld()
{
    sigset_t s_mask;
    sigemptyset(&s_mask);
    sigaddset(&s_mask, SIGCHLD);

    return pthread_sigmask(SIG_BLOCK, &s_mask, &SU_SRV_SIGMASK);
}

int pth_unblock_sigchld()
{
    return pthread_sigmask(SIG_SETMASK, &SU_SRV_SIGMASK, NULL);
}

void send_exit_command(su_shell_session *session)
{
    su_srv_log_cmdstr(SU_SRV_EXIT_CMD);
    write(session->afun_client_fd, SU_SRV_EXIT_CMD, strlen(SU_SRV_EXIT_CMD));
    write(session->afun_client_fd, "\n", 1);
}

void delete_current_session()
{
    if (acquire_handler_mutex(_su_session))
    {
        su_srv_log_printf(
                "Failed to acquire lock before deleting session, expect zombie mutexes !");
    }

    su_shell_session *session = _su_session;
    _su_session = NULL;
    su_shell_session_delete(session);
}

char *find_su_binary()
{
    struct stat su_bin_stat;
    int c=0;

    while ((c < sizeof(SU_SHELL_BINARY_CANDIDATES))
        && (stat(SU_SHELL_BINARY_CANDIDATES[c++], &su_bin_stat) != 0) );

    if (c == sizeof(SU_SHELL_BINARY_CANDIDATES))
        return NULL;
    
    return SU_SHELL_BINARY_CANDIDATES[c-1];
}

