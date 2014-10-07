#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <errno.h>

#include "su_srv_log.h"
#include "su_srv.h"

// Shared library (empty) constructor(s) and destructor(s)
//
void __attribute__ ((constructor)) libsusrv_init(void);
void libsusrv_init(void){}
void __attribute__ ((destructor)) libsusrv_dispose(void);
void libsusrv_dispose(void){}

// The local (UNIX) address of our su_srv
//
// sys/un.h
/*
 struct sockaddr_un
  {
    __SOCKADDR_COMMON (sun_);
    char sun_path[108];         
  };
*/
//
static struct sockaddr_un su_srv_un_addr;

// Path to the binary to exec on su_srv client connection
//
static char *su_shell_cmd = "/system/xbin/su";

// File descriptor of the shell client socket
//
static int su_shell_client_fd;

// POSIX thread context for su_srv socket listener
// 
static pthread_t su_srv_service_tid;
static pthread_mutex_t su_srv_ready_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t su_srv_ready_cond = PTHREAD_COND_INITIALIZER;

// POSIX thread context for shell commands handling
//
static pthread_t su_shell_session_tid;

// su_srv_init_fs(root): Initializes filesystem as
//
//  <root>/var/run  :to put local socket endpoints, locks, ...
//  <root>/var/log  :to put log files
//  <root>/bin      :to put bin files
//  <root>/llib     :to put shared libraries
// 
int su_srv_init_fs(const char * fs_root_path)
{
  char * dir_name;
  size_t path_len;
  char * path_ptstr;

  dir_name = "var";
  path_len = strlen(fs_root_path) + 1 + strlen(dir_name) + 1;
  path_ptstr = malloc(path_len);
  sprintf(path_ptstr, "%s/%s", fs_root_path, dir_name);
  mkdir(path_ptstr, 0700);
  free(path_ptstr);
  
  dir_name = "var/run";
  path_len = strlen(fs_root_path) + 1 + strlen(dir_name) + 1;
  path_ptstr = malloc(path_len);
  sprintf(path_ptstr, "%s/%s", fs_root_path, dir_name);
  mkdir(path_ptstr, 0700);
  free(path_ptstr);

  dir_name = "var/log";
  path_len = strlen(fs_root_path) + 1 + strlen(dir_name) + 1;
  path_ptstr = malloc(path_len);
  sprintf(path_ptstr, "%s/%s", fs_root_path, dir_name);
  mkdir(path_ptstr, 0700);
  free(path_ptstr);

  dir_name = "lib";
  path_len = strlen(fs_root_path) + 1 + strlen(dir_name) + 1;
  path_ptstr = malloc(path_len);
  sprintf(path_ptstr, "%s/%s", fs_root_path, dir_name);
  mkdir(path_ptstr, 0700);
  free(path_ptstr);

  dir_name = "bin";
  path_len = strlen(fs_root_path) + 1 + strlen(dir_name) + 1;
  path_ptstr = malloc(path_len);
  sprintf(path_ptstr, "%s/%s", fs_root_path, dir_name);
  mkdir(path_ptstr, 0700);
  free(path_ptstr);
  
  return 0;
}

//
// su_srv_service_thfunc():
// - setup local socket and listen
// - tells I'm ready
// - accepts one connection, fork the SU shell process, and redirect (dup2) its
//   stdin/stdout/stderr descriptors to the client socket
// - terminates this thread
//
// On error, the thread exit code points to the errno value if available.
//
void * su_srv_service_thfunc(void * targ)
{
  int server_fd=0, client_fd=0;
  unlink(su_srv_un_addr.sun_path);    
  socklen_t socklen = sizeof(struct sockaddr_un);
  
  server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_fd < 0) {
    su_srv_log_perror("Failed to open AF_UNIX socket stream, will abort");
    pthread_exit((void *) errno);
  }
    
  if (bind(server_fd,
        (struct sockaddr *) &su_srv_un_addr,
        sizeof(struct sockaddr_un)) < 0) {
    su_srv_log_perror("Failed to bind AF_UNIX socket, will abort");
    pthread_exit((void *) errno);
  }

  if (listen(server_fd, 1) < 0) {
    su_srv_log_perror("Failed to listen on AF UNIX socket, will abort");
    pthread_exit((void *) errno);
  }
  
  pthread_cond_signal(&su_srv_ready_cond);
  
  client_fd = accept(server_fd,
    (struct sockaddr *) &su_srv_un_addr,
    &socklen); 
  if (client_fd < 0) {
    su_srv_log_perror("Failed to accept client connection, will abort");
    pthread_exit((void *) errno);
  }
  
  pid_t su_shell_pid = fork();
  if (su_shell_pid < 0) {
    su_srv_log_perror("Failed to create shell process, will abort");
    pthread_exit((void *) errno);
  }

  if (su_shell_pid == 0) {
    char * const params[] = {su_shell_cmd, NULL};
    char * const environ[] = {
      "PATH=/sbin:/vendor/bin:/system/sbin:/system/bin:/system/xbin",
      NULL};

    dup2(client_fd,0);
    dup2(0,1);
    dup2(0,2);
    execve(su_shell_cmd, params, environ);          
  }
  else {
    su_srv_log_printf("SU shell PID: %d\n", su_shell_pid);
  }

  su_srv_log_printf("su_srv_service_thfunc(): exit\n");
  pthread_exit(0);  
}

//
// su_shell_session_thfunc():
// - uses su_srv_log_file_ptr as log file
// - reads shell session output and dumps it to log file
// - when after EOF, terminates this thread
//
// FIXME: should handle some return value back to caller
// (echo "srsuv: "$?)
//
// On error ..?
//
void * su_shell_session_thfunc(void * targ)
{
  int in_count;
  char buf[64];
  int fd = fileno(su_srv_log_file_ptr);
  
  while ((in_count = read(su_shell_client_fd, buf, 64)) != -1) {
    write(fd, buf, in_count);
  }

  pthread_exit(0);  
}

int su_srv_init(const char * appdir_path)
{
  // init local FS
  su_srv_init_fs(appdir_path);
  
  // init log
  if (su_srv_log_init(appdir_path) < 0)
    /* or not really, but harmless*/ ;

  // initializes local AF_UNIX address
  const char * un_name = "var/run/susrv_sock";
  size_t un_path_len = strlen(appdir_path) + 1 + strlen(un_name) + 1;
  char * un_path = malloc(un_path_len);
  sprintf(un_path, "%s/%s", appdir_path, un_name);
  strcpy(su_srv_un_addr.sun_path, un_path);
  free(un_path);
  su_srv_un_addr.sun_family = AF_UNIX;
  su_srv_log_printf("AF UNIX socket: %s\n", su_srv_un_addr.sun_path);
  
  
  // creates service thread
  if (pthread_create(&su_srv_service_tid,
    0,
    su_srv_service_thfunc,
    (void *) appdir_path)) {
      su_srv_log_printf("pthread_create() failed ?!\n");
      return -1;
  }
  // ... and waits till it's listening
  pthread_mutex_lock(&su_srv_ready_mutex);
  pthread_cond_wait(&su_srv_ready_cond, &su_srv_ready_mutex);
  pthread_mutex_unlock(&su_srv_ready_mutex);

  // connect back to get client socket (full duplex stream)
  su_shell_client_fd = socket(AF_UNIX, SOCK_STREAM, 0);    
  if (connect(su_shell_client_fd,
    (struct sockaddr *) &su_srv_un_addr,
    sizeof(struct sockaddr_un)) < 0) {
    su_srv_log_perror("Failed to connect to su_srv");
    return errno;
  }
  su_srv_log_printf("su_srv_init(%d)\n", su_shell_client_fd);
  // ... and waits till shell process initialization's complete
  void * thresult;
  pthread_join(su_srv_service_tid, &thresult);

  if ((int) thresult) {
    su_srv_log_printf("Failed to initialize SU shell process: %d\n", thresult);
    return (int) thresult;
  }
  else {
    su_srv_log_printf("SU shell process initialization's complete\n");
  }

  // starts shell session handler
  if (pthread_create(&su_shell_session_tid,
    0,
    su_shell_session_thfunc,
    (void *) 0)) {
      su_srv_log_printf("pthread_create() failed ?!\n");
      return -1;
  }
  else {
    su_srv_log_printf("SU session initialization's complete\n");
    su_srv_log_printf("------------------------------------\n");
  }

  return 0;
}

int su_srv_exe(const char *cmd_str)
{
  su_srv_log_printf("# %s\n", cmd_str);
  
  write(su_shell_client_fd, cmd_str, strlen(cmd_str));  
  write(su_shell_client_fd, "\n", 1);
  
  return 0;
}

// FIXME: for API compat
void su_srv_close() {}

// Output example:
/*
AF UNIX socket: /data/data/org.openmarl.talktoril/files/var/run/susrv_sock
su_srv_init(49)
SU shell PID: 7106
su_srv_service_thfunc(): exit
SU shell process initialization's complete
SU session initialization's complete
------------------------------------
# id
uid=0(root) gid=0(root) context=u:r:init:s0
*/
