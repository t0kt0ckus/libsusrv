#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "su_srv_log.h"

// var/log should exist 
static const char *LOG_FILE_NAME = "var/log/libsusrv.log";

int su_srv_log_init(const char * logdir_path)
{
  char * log_path = malloc(strlen(logdir_path) + strlen(LOG_FILE_NAME) +1 +1);
  if (sprintf(log_path, "%s/%s", logdir_path, LOG_FILE_NAME) < 0)
    return -1;
  
  su_srv_log_file_ptr = fopen(log_path, "w");
  free(log_path);

  return (su_srv_log_file_ptr == 0) ? -1 : 0;
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
    fprintf(su_srv_log_file_ptr, "%s: %s\n", msg, strerror(errno));
    fflush(su_srv_log_file_ptr);
  }
}    


