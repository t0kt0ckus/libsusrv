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

#define SESSION_BIN_DIR_FMT "%s/bin"
#define SESSION_BIN_DIR_WLEN 4

#define SESSION_LIB_DIR_FMT "%s/lib"
#define SESSION_LIB_DIR_WLEN 4

#define SESSION_TMP_DIR_FMT "%s/tmp"
#define SESSION_TMP_DIR_WLEN 4

static int pfs_init_dir(const char* pfs_root, 
        const char *dir_fmt, 
        int dir_wlen)
{
    int last_err = -1;
    char *dirpath;
    size_t dir_sz;
    struct stat dirstat;

    dir_sz = strlen(pfs_root) + dir_wlen + 1;
    if ( (dirpath = malloc(sizeof(char) * dir_sz)) )
    {
        snprintf(dirpath, dir_sz, dir_fmt, pfs_root);
        if (stat(dirpath, &dirstat))
            mkdir(dirpath, 0700);

        if (stat(dirpath, &dirstat))
            last_err = errno;
        else
            last_err = 0;

        free(dirpath);
    }
    return last_err;
}

int su_srv_pfs_init(const char *pfs_root)
{
    return pfs_init_dir(pfs_root, SESSION_VAR_DIR_FMT, SESSION_VAR_DIR_WLEN)
        || pfs_init_dir(pfs_root, SESSION_RUN_DIR_FMT, SESSION_RUN_DIR_WLEN)
        || pfs_init_dir(pfs_root, SESSION_LOG_DIR_FMT, SESSION_LOG_DIR_WLEN)
        || pfs_init_dir(pfs_root, SESSION_BIN_DIR_FMT, SESSION_BIN_DIR_WLEN)
        || pfs_init_dir(pfs_root, SESSION_LIB_DIR_FMT, SESSION_LIB_DIR_WLEN)
        || pfs_init_dir(pfs_root, SESSION_TMP_DIR_FMT, SESSION_TMP_DIR_WLEN);
}

