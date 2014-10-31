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

#include "su_shell_tools.h"

#define PROC_TABLE_SIZE 32
#define PROC_TABLE_INC 16

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
                // FIXME: we use cmdline as Android truncs /proc/<pid>/comm
                snprintf(proc_entry_path,
                        sizeof(proc_entry_path), 
                        "/proc/%ld/cmdline",
                        lpid);
                proc_entry_file = fopen(proc_entry_path, "r");

                if (proc_entry_file)
                {
                    if ((fgets(proc_entry_str,
                                    sizeof(proc_entry_str),
                                    proc_entry_file)))
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

int su_srv_getproclist(proc_def_t **proc_table_ptr)
{
    proc_def_t *proc_table = calloc(PROC_TABLE_SIZE, sizeof(proc_def_t));

    DIR *dir_ptr;
    struct dirent *dirent_ptr;
    char *endptr;
    
    char proc_entry_path[255];
    FILE *proc_entry_file;
    char proc_entry_str[255];

    int nproc = 0;
    int proc_table_n = PROC_TABLE_SIZE;

    if ( (dir_ptr = opendir("/proc")) ) 
    {
        while ((dirent_ptr = readdir(dir_ptr)) != NULL)
        {
            long lpid = strtol(dirent_ptr->d_name, &endptr, 10);
            if (*endptr == 0)
            {
                // FIXME: we use cmdline as Android truncs /proc/<pid>/comm
                snprintf(proc_entry_path,
                        sizeof(proc_entry_path), "/proc/%ld/cmdline",
                        lpid);
                proc_entry_file = fopen(proc_entry_path, "r");

                if (proc_entry_file)
                {
                    if ((fgets(proc_entry_str,
                                    sizeof(proc_entry_str), proc_entry_file)))
                    {
                        if (proc_entry_str[strlen(proc_entry_str)-1] == 10)
                            proc_entry_str[strlen(proc_entry_str)-1] = 0;

                        if (! (nproc < proc_table_n)) 
                        {
                            proc_table_n  += PROC_TABLE_INC;
                            proc_table = realloc(proc_table,
                                    proc_table_n * sizeof(proc_def_t));
                        }

                        proc_def_t *procdef = &proc_table[nproc++];
                        procdef->pid = lpid;
                        procdef->comm = malloc(
                                (strlen(proc_entry_str) + 1) * sizeof(char));
                        strcpy(procdef->comm, proc_entry_str);
                    }
                    
                    fclose(proc_entry_file);
                }
            }
        }
        closedir(dir_ptr);
    }

    *proc_table_ptr = proc_table;
    return nproc;
}


