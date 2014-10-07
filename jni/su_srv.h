/*
 * libsusrv: Android SU native client library.
 *
 * Chris Dufaza
 * <t0kt0ckus@gmail.com>
 * 
 * (C) 2014
 *
 * License: GPLv3
 *
 */
#ifndef _SU_SRV_H
#define _SU_SRV_H

#ifdef __cplusplus
extern "C" {
#endif

//
// su_srv_init(): Initializes the SU shell session.
//
// - init log
// - setup UNIX rendez-vous socket and listen
// - initiates local client connection
// - accepts the connection, forks an SU shell process, and redirect
//   (dup2) its stdin/stdout/stderr descriptors to the client socket
// - starts the shell session handler thread
// - close unused files
// 
// Returns: 0 on success, -1 on failure.
//
int su_srv_init(const char * appdir_path);

//
// su_srv_exe(): Executes a shell command
//
//
// Returns: the shell command result code, or SU_SRV_CMD_FAILED when
//          command string execution failed
//
int su_srv_exe(const char *cmd_str);
//
int SU_SRV_CMD_FAILED;


//
// su_srv_dispose(): Releases resources and close session
//
void su_srv_close();


#ifdef __cplusplus
}
#endif

#endif
