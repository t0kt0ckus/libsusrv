#ifndef _SU_SRV_H
#define _SU_SRV_H

#ifdef __cplusplus
extern "C" {
#endif

// su_srv_init: Initializes SU shell.
//
// Returns a negative value on error.
//
int su_srv_init(const char * appdir_path);

// su_srv_exe: Pipes a command to shell process.
//
// FIXME: currently always returns 0 instead of command exit code. 
//
int su_srv_exe(const char *cmd_str);

#ifdef __cplusplus
}
#endif

#endif
