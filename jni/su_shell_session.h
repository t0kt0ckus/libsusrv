/*
 * libsusrv: Android SU native client library.
 *
 * <t0kt0ckus@gmail.com>
 * (C) 2014
 *
 * License: GPLv3
 *
 */
#ifndef _SU_SHELL_SESSION_H
#define _SU_SHELL_SESSION_H

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/un.h>

#define AF_UNIX_PATH_MAX 108

#ifdef __cplusplus
extern "C" {
#endif

/// Represents a SU shell session.
///
typedef struct su_shell_session 
{   
    // PID of the owner process, which is also the session identifier
    //
    pid_t owner_pid;

    // Absolute path to the private filesystem
    //
    const char *pfs_root;

    // AF_UNIX address of the rendez-vous socket
    //
    struct sockaddr_un afun_rdv_addr;

    // Descriptor of this session AF_UNIX/SOCKSTREAM client socket
    //
    int afun_client_fd;

    // PID of the shell subprocess
    //
    pid_t shell_pid;

    // POSIX thread context of this shell session
    //
    pthread_t *handler_pth;
    //
    pthread_mutex_t *shell_sync_mutex;
    pthread_cond_t *shell_sync_ready;
    //
    int echo;
    char *handler_buf;

    // last line read on "terminal"
    char *last_tty_read;

    // Last executed command exit code
    //
    int shell_cmd_exit_code;

} su_shell_session;

/// Allocates/initializes a new session.
/// 
/// Returns: a pointer to the new session, NULL on failure.
///
su_shell_session *su_shell_session_new(const char * pfs_root, pid_t owner_pid);

/// AF UNIX rendez-vous.
///
/// Returns: the accepted connection's file descriptor on success,
/// a negative value on failure.
///
int su_shell_session_af_un_rdv(su_shell_session *session);

/// Destroys a session
///
void su_shell_session_delete(su_shell_session *session);


#ifdef __cplusplus
}
#endif

#endif
