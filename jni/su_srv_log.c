/*
 * libsusrv: Android SU native client library.
 *
 * <t0kt0ckus@gmail.com>
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

static const char *LOG_FILE_NAME = "var/log/libsusrv.log";

int su_srv_log_init(const char * appdir_path)
{
    size_t log_path_len = sizeof(char) * (strlen(appdir_path) + 1 + strlen(LOG_FILE_NAME) +1);
    char * log_path = malloc(log_path_len);
        
    if (sprintf(log_path, "%s/%s", appdir_path, LOG_FILE_NAME) < 0)
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

void su_srv_log_print_header()
{
    fprintf(su_srv_log_file_ptr, "[su_srv] ");
}

void su_srv_log_print_footer() 
{
    fprintf(su_srv_log_file_ptr,"\n");
    fflush(su_srv_log_file_ptr);
}

void su_srv_log_printf(const char *fmt, ...)
{
    if (su_srv_log_file_ptr)
    {
        su_srv_log_print_header();
        va_list argp;
        va_start(argp, fmt);
        vfprintf(su_srv_log_file_ptr, fmt, argp);
        va_end(argp);
        su_srv_log_print_footer();
    }    
}

void su_srv_log_perror(const char *msg)
{
  if (su_srv_log_file_ptr)
  {
      su_srv_log_print_header();  
      fprintf(su_srv_log_file_ptr, "[errno: %d] %s (%s)", 
              errno, msg,
              strerror(errno));
      su_srv_log_print_footer();
  }
}    

// su_srv_log_cmdstr():
//
void su_srv_log_cmdstr(const char *cmdstr)
{
    if (su_srv_log_file_ptr)
    {
        fprintf(su_srv_log_file_ptr , "# %s\n", cmdstr);
        fflush(su_srv_log_file_ptr);
    }
}

