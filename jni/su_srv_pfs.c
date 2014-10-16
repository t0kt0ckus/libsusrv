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
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "su_srv_pfs.h"

///////////////////////////////////////////////////////////////////////////////////////////////
//
// Constants
//

#define SESSION_VAR_DIR_FMT "%s/var"
#define SESSION_VAR_DIR_WLEN 4

#define SESSION_LOG_DIR_FMT "%s/var/log"
#define SESSION_LOG_DIR_WLEN 8

#define SESSION_RUN_DIR_FMT "%s/var/run"
#define SESSION_RUN_DIR_WLEN 8


// su_srv_pfs_init():
//
int su_srv_pfs_init(const char *pfs_root)
{
    int last_err = -1;
    char *dirpath;
    size_t dir_sz;
    struct stat dirstat;

    dir_sz = strlen(pfs_root) + SESSION_VAR_DIR_WLEN + 1;
    if ( (dirpath = malloc(sizeof(char) * dir_sz)) )
    {
        snprintf(dirpath, dir_sz, SESSION_VAR_DIR_FMT, pfs_root);
        if (stat(dirpath, &dirstat))
            mkdir(dirpath, 0700);

        if (stat(dirpath, &dirstat))
            last_err = errno;
        else
            last_err = 0;
        free(dirpath);
    }
    
    if (! last_err)
    {
        dir_sz = strlen(pfs_root) + SESSION_LOG_DIR_WLEN + 1;
        if ( (dirpath = malloc(sizeof(char) * dir_sz)) )
        {
            snprintf(dirpath, dir_sz, SESSION_LOG_DIR_FMT, pfs_root);
            if (stat(dirpath, &dirstat))
                mkdir(dirpath, 0700);

            if (stat(dirpath, &dirstat))
                last_err = errno;
            else
                last_err = 0;
            free(dirpath);
        }
    }

    if (! last_err)
    {
        dir_sz = strlen(pfs_root) + SESSION_RUN_DIR_WLEN + 1;
        if ( (dirpath = malloc(sizeof(char) * dir_sz)) )
        {
            snprintf(dirpath, dir_sz, SESSION_RUN_DIR_FMT, pfs_root);
            if (stat(dirpath, &dirstat))
                mkdir(dirpath, 0700);
            if (stat(dirpath, &dirstat))
                last_err = errno;
            free(dirpath);
        }
    } 
    
    return last_err;
}

