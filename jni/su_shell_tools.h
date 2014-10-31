/*
 * libsusrv: Android SU native client library.
 *
 * <t0kt0ckus@gmail.com>
 * (C) 2014
 *
 * License: GPLv3
 *
 */
#ifndef _SU_SHELL_TOOLS_H
#define _SU_SHELL_TOOLS_H

long su_srv_getpid(const char *procname);

typedef struct proc_def {
    long pid;
    char *comm;
} proc_def_t;

int su_srv_getproclist(proc_def_t **proc_table_ptr);

#endif
