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
        "/bin/su", "/sbin/su", "/usr/bin/su", "/usr/sbin/su", NULL
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
static const int WAIT_CHILD_SLEEP_SEC = 1;


/////////////////////////////////////////////////////////////////////////////////////////////
//
// FORWARD definition
//

static char *find_su_binary();

static int pth_block_sigchld();
static int pth_unblock_sigchld();

static void *session_handler_fn(void *);

static int create_handler_thread(su_shell_session *session, void *(* handler_fn)(void *));

static int start_shell_process(int fd, su_shell_session *session);
static int stop_shell_process(su_shell_session *session);

static int destroy_session(su_shell_session *session);

///////////////////////////////////////////////////////////////////////////////////////////////
//
//                                         State

static su_shell_session *_su_session = NULL;
static sigset_t SU_SRV_SIGMASK;


///////////////////////////////////////////////////////////////////////////////////////////////
//
// API: su_session.h
//
///////////////////////////////////////////////////////////////////////////////////////////////

int su_srv_open_shell_session(const char *pfs_root)
{
    if (_su_session)
        return SU_SRV_SESSION_ALREADY_OPEN_ERROR;

    if (su_srv_pfs_init(pfs_root))
        return SU_SRV_PFS_ERROR;

    pid_t owner_pid = getpid();
    su_shell_session *session;
   
    su_srv_log_init(pfs_root, owner_pid);

    if ((session = su_shell_session_new(pfs_root, owner_pid)) == NULL)
        return SU_SRV_SYSTEM_ERROR;

    su_srv_log_printf("Initializing SU shell session:");
    su_srv_log_printf("owner PID: %d", session->owner_pid);
    su_srv_log_printf("AF UNIX path: %s", session->afun_rdv_addr.sun_path);

    int last_err = 0;

    int peer_fd = su_shell_session_af_un_rdv(session);
    if (peer_fd > 0)
    {
        su_srv_log_printf("AF UNIX rendez-vous complete");
        if ((last_err = start_shell_process(peer_fd, session)))
        {
            su_srv_log_printf("Failed to create SU shell child process");
        }
        else
        {
            su_srv_log_printf("Created SU shell child process (PID: %d)", session->shell_pid);

            if ((last_err = create_handler_thread(session, session_handler_fn)))
            {
                su_srv_log_printf("Failed to create handler loop's thread !");

                // when unable to start associated handler loop, stops the shell process
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
    if (! _su_session)
    {
        su_srv_log_printf("Ignored exit(), no session");
        return SU_SRV_NO_SESSION_ERROR;
    }

    int last_err = pthread_mutex_trylock(_su_session->shell_sync_mutex);
    if (last_err)
    {
        pthread_cond_signal(_su_session->shell_sync_ready);
        last_err = pthread_mutex_lock(_su_session->shell_sync_mutex);
    }

    if (last_err) 
        su_srv_log_printf("Failed to acquire lock on exit()");
    else
    {
        su_shell_session *session_to_kill = _su_session;
        _su_session = NULL;
        destroy_session(session_to_kill); // destroy will unlock mutex 
    }

    return last_err;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// FORWARDs implementation
//


void *session_handler_fn(void * targs)
{
    int fd = fileno(su_srv_log_fptr);
      
    char * su_shell_linebuf_ptr = malloc(64 * sizeof(char));
    int curr_line_offset = 0;
    char curr_byte;

    while (read(_su_session->afun_client_fd, &curr_byte, 1) > 0) 
    {
        if (curr_line_offset > (sizeof(su_shell_linebuf_ptr -1 -1) ))
        {
            su_shell_linebuf_ptr = realloc(su_shell_linebuf_ptr,
                                           (curr_line_offset + 64) * sizeof(char));
        }
        su_shell_linebuf_ptr[curr_line_offset++] = curr_byte;

        if (curr_byte == 10)
        {
            su_shell_linebuf_ptr[curr_line_offset] = 0;
            curr_line_offset = 0;

            if (strstr(su_shell_linebuf_ptr, EOC_TAG))
            {
                if (sscanf(su_shell_linebuf_ptr,
                            EOC_SSCAN,
                            &_su_session->shell_cmd_exit_code) == EOF)
                {
                    _su_session->shell_cmd_exit_code = SU_SRV_EXIT_CODE_UNKNOWN;
                }

                pthread_cond_signal(_su_session->shell_sync_ready);          
            }
            else
            {
                write(fd, su_shell_linebuf_ptr, strlen(su_shell_linebuf_ptr));
            }      
        }
    }

    free(su_shell_linebuf_ptr);
    pthread_exit(0);  
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

    pid_t pid = fork();
    if (pid < 0)
    {
        su_srv_log_perror("Failed to fork()");
        return SU_SRV_SYSTEM_ERROR;
    }
    else if (pid == 0)
    {
        char *const params[2] = {su_binary, NULL};

        dup2(fd,0);
        dup2(1,0);
        dup2(2,0);
        execve(su_binary, params, SU_SHELL_ENVIRONMENT);
    }
    else
    {
        session->shell_pid = pid;
    }

    return 0;
}

int stop_shell_process(su_shell_session *session)
{
    int waited = 0;

    if (session->shell_pid)
    {
        int again = 0;
        int wstatus;
         
        while ((again < WAIT_CHILD_MAX_RETRY) && (! waited))
        {
            // note: we use waitpid/sleep as sigtimedwait's not defined by Android libc
            
            if ((waited = waitpid(session->shell_pid, &wstatus, WNOHANG)) < 0)
            {
                if (errno == ECHILD)
                {
                    // ok, already dead
                    waited = session->shell_pid; 
                }
                else 
                {
                    // retry
                    waited = 0;
                    if (again++ == 0)
                    {
                        // try to KILL on 1rst retry
                        su_srv_log_printf("EAGAIN: kill(%d, SIGKILL)", session->shell_pid);
                        kill(session->shell_pid, SIGKILL);
                    }
                    
                    su_srv_log_printf("EAGAIN: sleep ...");
                    sleep(WAIT_CHILD_SLEEP_SEC);
                }
            }
            else
            {
                // shell process killed/waited
                waited = session->shell_pid; 
                session->shell_pid = 0;
            }

        } // end-while-not-waited 
        
    } // end-shell-pid    
    
    return (waited > 0) ? 0 : -1;
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

int destroy_session(su_shell_session *session)
{
    int last_err;

     // sends the exit command to shell process
    su_srv_log_cmdstr(SU_SRV_EXIT_CMD);
    write(session->afun_client_fd, SU_SRV_EXIT_CMD, strlen(SU_SRV_EXIT_CMD));
    write(session->afun_client_fd, "\n", 1);

    // try to grant shell process closed
    if ((last_err = stop_shell_process(session)))
        su_srv_log_printf("Failed to stop an SU shell, expect a zombie process");
    else
    {
        su_srv_log_printf("SU shell process stopped");
        if ((! last_err) && (last_err = pthread_join(*_su_session->handler_pth, NULL)))
            su_srv_log_printf(
                        "Failed to stop a session handler thread, expect a zombie thread");
        else
            su_srv_log_printf("Session handler thread stopped");
    }

    su_shell_session_delete(session); // will unlock mutex

    return last_err;
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

