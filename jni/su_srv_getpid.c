/*
 * libsusrv: Android SU native client library.
 *
 * <t0kt0ckus@gmail.com>
 * (C) 2014
 *
 * License: GPLv3
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "su_srv_getpid.h"

long su_srv_getpid(const char *procname)
{
    long procpid = -1;

    DIR *dir_ptr;
    struct dirent *dirent_ptr;
    char *endptr;
    
    char proc_entry_path[255];
    FILE *proc_entry_file;
    char proc_entry_str[255];
    
    if ( (dir_ptr = opendir("/proc")) ) 
    {
        while ((procpid < 0) && ((dirent_ptr = readdir(dir_ptr)) != NULL))
        {
            long lpid = strtol(dirent_ptr->d_name, &endptr, 10);
            if (*endptr == 0)
            {
                snprintf(proc_entry_path, sizeof(proc_entry_path), "/proc/%ld/comm", lpid);
                proc_entry_file = fopen(proc_entry_path, "r");

                if (proc_entry_file)
                {
                    if ((fgets(proc_entry_str, sizeof(proc_entry_str), proc_entry_file)))
                    {
                        if (proc_entry_str[strlen(proc_entry_str)-1] == 10)
                            proc_entry_str[strlen(proc_entry_str)-1] = 0;

                        if (! strcmp(procname, proc_entry_str))
                            procpid = lpid;
                    }
                    fclose(proc_entry_file);
                }
            }
        }
        closedir(dir_ptr);
    }
    
    return procpid;
}
