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
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "su_srv_log.h"

FILE *su_srv_log_fptr;

static const char *SU_SRV_LOG_FMT = "%s/var/log/su-%05d.log";
static const int SU_SRV_LOG_WLEN = 16 + 5;

int su_srv_log_init(const char * pfs_root, int pid)
{
    int last_err = -1;
    int logpath_len = strlen(pfs_root) + SU_SRV_LOG_WLEN + 1;
    char *logpath;
    
    if ((logpath = malloc(sizeof(char) * logpath_len)))
    {
        snprintf(logpath, logpath_len, SU_SRV_LOG_FMT, pfs_root, pid);
        if ((su_srv_log_fptr = fopen(logpath, "a")))
            last_err = 0;
        else
            last_err = errno;
        free(logpath);
    }

    return last_err;
}

void su_srv_log_close()
{
    if (su_srv_log_fptr) {
        fclose(su_srv_log_fptr);
        su_srv_log_fptr = NULL;
    }
}

void su_srv_log_print_header()
{
    fprintf(su_srv_log_fptr, "[su_srv] ");
}

void su_srv_log_print_footer() 
{
    fprintf(su_srv_log_fptr,"\n");
    fflush(su_srv_log_fptr);
}

void su_srv_log_printf(const char *fmt, ...)
{
    if (su_srv_log_fptr)
    {
        su_srv_log_print_header();
        va_list argp;
        va_start(argp, fmt);
        vfprintf(su_srv_log_fptr, fmt, argp);
        va_end(argp);
        su_srv_log_print_footer();
    }    
}

void su_srv_log_perror(const char *msg)
{
  if (su_srv_log_fptr)
  {
      su_srv_log_print_header();  
      fprintf(su_srv_log_fptr, "! %s: %s (%d)",
              msg,              
              strerror(errno),
              errno);
      su_srv_log_print_footer();
  }
}    

void su_srv_log_cmdstr(const char *cmdstr)
{
    if (su_srv_log_fptr)
    {
        fprintf(su_srv_log_fptr, "# %s\n", cmdstr);
        fflush(su_srv_log_fptr);
    }
}

