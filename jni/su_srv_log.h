/*
 * libsusrv: Android SU native client library.
 *
 * <t0kt0ckus@gmail.com>
 * (C) 2014
 *
 * License: GPLv3
 *
 */
#ifndef _SU_SRV_LOG_H
#define _SU_SRV_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

FILE *su_srv_log_file_ptr;

// su_srv_log_init(): initializes the log file
// 
// fs_path: the log file will be <fs_path>/var/log/libsusrv.log (<fs_path>/var/log must exist)
//
// Returns: 0 on success, negative value on error.
//
int su_srv_log_init(const char * fs_path);

// su_srv_close():
//
void su_srv_log_close();

// su_srv_log_printf():
//
void su_srv_log_printf(const char *fmt, ...);

// su_srv_log_perror():
//
void su_srv_log_perror(const char *msg);

// su_srv_log_cmdstr():
//
void su_srv_log_cmdstr(const char *cmdstr);

#ifdef __cplusplus
}
#endif

#endif
