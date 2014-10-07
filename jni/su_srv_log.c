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
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "su_srv_log.h"

// var/log should exist within appdir_path
static const char *LOG_FILE_NAME = "var/log/libsusrv.log";

int su_srv_log_init(const char * appdir_path)
{
  char * log_path = malloc(strlen(appdir_path) + strlen(LOG_FILE_NAME) +1 +1);
  if (sprintf(log_path, "%s/%s", appdir_path, LOG_FILE_NAME) < 0)
    return -1;
  
  su_srv_log_file_ptr = fopen(log_path, "w");
  free(log_path);

  return (su_srv_log_file_ptr == 0) ? errno : 0;
}

void su_srv_log_close()
{
  if (su_srv_log_file_ptr) {
    fclose(su_srv_log_file_ptr);
    su_srv_log_file_ptr = (FILE *) 0;
  }
}

void su_srv_log_printf(const char *fmt, ...)
{
  if (su_srv_log_file_ptr) {
    va_list argp;
    va_start(argp, fmt);
    vfprintf(su_srv_log_file_ptr, fmt, argp);
    va_end(argp);
    fflush(su_srv_log_file_ptr);
  }    
}

void su_srv_log_perror(const char *msg)
{
  if (su_srv_log_file_ptr) {
    fprintf(su_srv_log_file_ptr, "%s: %s [%d]\n", msg, strerror(errno),
            errno);
    fflush(su_srv_log_file_ptr);
  }
}    


