#ifndef _SU_SRV_LOG_H
#define _SU_SRV_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

FILE *su_srv_log_file_ptr;

// Initializes the log file in var/log relative to the given directory.
//
// Returns: negative value on error.
//
int su_srv_log_init(const char * fs_path);

// Simply closes the log file if exists.
//
void su_srv_log_close();

// Simple log functions
//
void su_srv_log_printf(const char *fmt, ...);
void su_srv_log_perror(const char *msg);


#ifdef __cplusplus
}
#endif

#endif
