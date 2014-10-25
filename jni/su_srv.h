/*
 * libsusrv: Android SU native client library.
 *
 * <t0kt0ckus@gmail.com>
 * (C) 2014
 *
 * License: GPLv3
 *
 */
#ifndef _SU_SRV_H
#define _SU_SRV_H

#define SU_SRV_SYSTEM_ERR (-9999)
#define SU_SRV_SESSION_EXISTS_ERR (-9998)
#define SU_SRV_NO_SESSION_ERR (-9997)

#define SU_SRV_INVALID_RESULT_ERR (-9996)

#define SU_SRV_EXIT_CMD "exit"

#ifdef __cplusplus
extern "C" {
#endif

/// Attempts to bind a new SU shell session to the requesting process.
///
/// <pfs_root>: root of the shell private filesystem.
///
/// This call initializes:
/// + <pfs_root>/var        (0700)
/// + <pfs_root>/var/log    (0700)
/// + <pfs_root>/var/run    (0700)
/// + <pfs_root>/bin        (0700)
/// + <pfs_root>/lib        (0700)
/// + <pfs_root>/tmp        (0700)
///
/// All directories above are owned by the UID of the requesting application,
/// not root (0).
///
/// Returns: 0, when a new session was successfully initialized,
///          SU_SRV_SESSION_EXISTS_ERR when a session is already bound,
///          or an error code on failure.
///
int su_srv_open_shell_session(const char * pfs_root);

/// Executes a command string within the SU shell session currently
/// bound to this process.
///
/// Returns: SU_SRV_NO_SESSION_ERR when no shell session's currently bound,
///          or SU_SRV_INVALID_RESULT_ERR when the command has been aborted,
///          or the (should be non negative,or at least > -1) command exit code.
///
int su_srv_exec(const char *cmd_str);

/// Exits the shell session currently bound to this process, if any.
///
/// Returns: O, when an existing session has been successfully closed,
///          or SU_SRV_SYSTEM_ERR when failed to close an existing session,
///          or SU_SRV_NO_SESSION_ERR when no session currently bound.
///
int su_srv_exit_shell_session();

/// Tells whether a shell session is currently bound to this process.
///
int su_srv_shell_session_ping();

#ifdef __cplusplus
}
#endif

#endif
