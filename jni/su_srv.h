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

#define SU_SRV_SYSTEM_ERROR (-9999)
#define SU_SRV_PFS_ERROR (-9998)
#define SU_SRV_SESSION_ALREADY_OPEN_ERROR (-9997)
#define SU_SRV_NO_SESSION_ERROR (-9996)
#define SU_SRV_NO_SYSTEM_SU_ERROR (-9995)

#define SU_SRV_EXIT_CODE_UNKNOWN (-9999)

#define SU_SRV_EXIT_CMD "exit"

#ifdef __cplusplus
extern "C" {
#endif

/// su_srv_open_shell_session(): Attempts to open the SU shell session
///                              owned by the this process.
///
// Returns: 0, when a new session was successfully initialized.
// Returns: SU_SRV_SESSION_ALREADY_OPEN_ERROR, when this process already owns a session.
// Returns: SU_SRV_PFS_ERROR, when failed to initialize the session's private filesystem.
// Returns: SU_SRV_SYSTEM_ERROR otherwise.
//
int su_srv_open_shell_session(const char * pfs_root);

/// su_srv_exe(): Executes a shell command within the session owned by this process.
///
/// Returns: on success, the shell command result code
/// Returns: SU_SRV_NO_SESSION_ERROR when this process does not own any SU shell session.
/// Returns: SU_SRV_EXIT_CODE_UNKNOWN otherwise.
///
int su_srv_exec(const char *cmd_str);

/// su_srv_exit_shell_session(): Exits the shell session currently owned by this process, if any.
//
int su_srv_exit_shell_session();


#ifdef __cplusplus
}
#endif

#endif
