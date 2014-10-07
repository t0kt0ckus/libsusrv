/*
 * libsusrv: Android SU native client library.
 *
 * Chris Dufaza
 * <t0kt0ckus@gmail.com>
 * 
 * (C) 2014
 *
 * License: GPLv3
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <errno.h>

#include "su_srv_log.h"
#include "su_srv.h"

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

// SU shell definition
// (should work on most Android/SU)
//
static const char *su_shell_cmd = "/system/xbin/su";
static char * const su_shell_params[] = {"/system/xbin/su", NULL};
static char * const su_shell_environment[] = {
      "PATH=/sbin:/vendor/bin:/system/sbin:/system/bin:/system/xbin",
      NULL};

// File descriptors of the UNIX socket
//
static int un_rdv_fd = 0, un_peer_fd = 0;
socklen_t un_socklen = sizeof(struct sockaddr_un);


// File descriptor of our shell client UNIX socket
//
static int su_shell_client_fd;

// POSIX thread context for shell session handler
//
static pthread_t su_shell_session_tid;
// mutex to synchronize result code extraction
static pthread_mutex_t su_shell_command_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t su_shell_command_cond = PTHREAD_COND_INITIALIZER;
static int su_shell_cmd_result_code;

// cleanup_rdv(): closes rdv files
// 
void cleanup_rdv() {

  if (un_rdv_fd > 0) {
    close(un_rdv_fd);
    un_rdv_fd = 0;
  }
  if (un_peer_fd > 0) {    
    close(un_peer_fd);
    un_peer_fd = 0;
  }
}

// su_shell_session_thfunc(): shell session handler thread that
//                            is responsible for result code extraction
//
//
static const char * EOC_CMD = "echo \"<EOC>\"$?\n";
static const char * EOC_TAG = "<EOC>";
static const char * EOC_SSCAN ="<EOC>%d\n";
int SU_SRV_CMD_FAILED = -999;
//
void * su_shell_session_thfunc(void * targ)
{
  int fd = fileno(su_srv_log_file_ptr);
  
  char * session_line_buf = malloc(64 * sizeof(char));
  int curr_line_offset = 0;
  char curr_byte;
  
  while (read(su_shell_client_fd, &curr_byte, 1) != -1) {
    
    if (curr_line_offset > (sizeof(session_line_buf) -1 -1) ) {
      session_line_buf = realloc(session_line_buf,
                                 (curr_line_offset + 64) * sizeof(char));
    }

    session_line_buf[curr_line_offset++] = curr_byte;

    // end of line
    if (curr_byte == 10) {
      session_line_buf[curr_line_offset] = 0;
      curr_line_offset = 0;

      if (strstr(session_line_buf, EOC_TAG)) {
      // the line seems to be an EAC tag
        if (sscanf(session_line_buf,
                   EOC_SSCAN,
                   &su_shell_cmd_result_code) == EOF) {
          su_shell_cmd_result_code = SU_SRV_CMD_FAILED;
        }
        pthread_cond_signal(&su_shell_command_cond);          
      }
      else {
      // the line seems to be a proper shell output
        write(fd, session_line_buf, strlen(session_line_buf));
      }      
    }
    
  }

  free(session_line_buf);
  pthread_exit(0);  
}

//
// su_srv_init(): Initializes the SU shell session.
//
// - init log
// - setup UNIX rendez-vous socket and listen
// - initiates local client connection
// - accepts the connection, forks an SU shell process, and redirect
//   (dup2) its stdin/stdout/stderr descriptors to the client socket
// - starts the shell session handler thread
// - close unused files
// 
// Returns: 0 on success, -1 on failure.
//
int su_srv_init(const char * appdir_path)
{  
  // init log
  if (su_srv_log_init(appdir_path))
    su_srv_log_printf("Initializing libsusrv ...\n")
    /* or not really, but harmless */;

  // initializes AF_UNIX address
  size_t un_path_len = strlen(appdir_path) + 1 + strlen("var/run/susrv_sock") + 1;
  if (un_path_len > 108) {
    return -1;
  }  
  char * un_path = malloc(un_path_len);
  sprintf(un_path, "%s/%s", appdir_path, "var/run/susrv_sock");
  strcpy(su_srv_un_addr.sun_path, un_path);
  free(un_path);
  su_srv_un_addr.sun_family = AF_UNIX;

  // creates the rendez-vous UNIX socket and starts listening
  unlink(su_srv_un_addr.sun_path);    
  un_rdv_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (un_rdv_fd < 0) {
    su_srv_log_perror("Failed to open AF_UNIX socket stream, will abort");
    return -1;
  }    
  if (bind(un_rdv_fd,
           (struct sockaddr *) &su_srv_un_addr,
           sizeof(struct sockaddr_un)) < 0) {
    su_srv_log_perror("Failed to bind AF_UNIX socket, will abort");
    return -1;
  }
  if (listen(un_rdv_fd, 1) < 0) {
    su_srv_log_perror("Failed to listen on AF UNIX socket, will abort");
    return -1;
  }
  su_srv_log_printf("Initialized UNIX socket: %s\n",
                    su_srv_un_addr.sun_path);

  // initiates the client connection to this local socket
  su_shell_client_fd = socket(AF_UNIX, SOCK_STREAM, 0);      
  if (connect(su_shell_client_fd,
              (struct sockaddr *) &su_srv_un_addr,
              sizeof(struct sockaddr_un)) < 0) {      
    su_srv_log_perror("Failed to connect to su_srv");    
    return -1;
  }

  // accepts client connection and initializes peer socket
  un_peer_fd = accept(un_rdv_fd,
                      (struct sockaddr *) &su_srv_un_addr,
                      &un_socklen); 
  if (un_peer_fd < 0) {
    su_srv_log_perror("Failed to accept client connection, will abort");
    return -1;
  }
  // ... and redirect in/out/err to child SU shell process
  pid_t su_shell_pid = fork();
  if (su_shell_pid < 0) {
    su_srv_log_perror("Failed to create shell process, will abort");
    return -1;
  }
  if (su_shell_pid == 0) {
    dup2(un_peer_fd,0);
    dup2(0,1);
    dup2(0,2);
    execve(su_shell_cmd, su_shell_params, su_shell_environment);          
  }
  else {
    su_srv_log_printf("SU shell PID: %d\n", su_shell_pid);

    // starts shell session handler
    if (pthread_create(&su_shell_session_tid,
                       0,
                       su_shell_session_thfunc,
                       (void *) 0)) {
      su_srv_log_printf("pthread_create() failed ?!\n");
      return -1;
    }
    else {
      //cleanup_rdv();
      su_srv_log_printf("SU session initialization's complete\n");
      su_srv_log_printf("------------------------------------\n");
    }
  }
  
  return 0;
}

//
// su_srv_exe(): Executes a shell command
//
//
// Returns: the shell command result code, or SU_SRV_CMD_FAILED when
//          command string execution failed
//
int su_srv_exe(const char *cmd_str)
{
  if (su_shell_client_fd > 0) {
    su_srv_log_printf("# %s\n", cmd_str);
    
    write(su_shell_client_fd, cmd_str, strlen(cmd_str));  
    write(su_shell_client_fd, "\n", 1);

    write(su_shell_client_fd, EOC_CMD, strlen(EOC_CMD));  
    write(su_shell_client_fd, "\n", 1);
        
    pthread_mutex_lock(&su_shell_command_mutex);
    pthread_cond_wait(&su_shell_command_cond, &su_shell_command_mutex);
    pthread_mutex_unlock(&su_shell_command_mutex);

    return su_shell_cmd_result_code;
  }
  else {
    su_srv_log_printf("Not connected, command canceled: %s\n", cmd_str);
    return -1;
  }
}

void su_srv_exe_nowait(const char *cmd_str)
{
  if (su_shell_client_fd > 0) {
    su_srv_log_printf("# %s\n", cmd_str);
    
    write(su_shell_client_fd, cmd_str, strlen(cmd_str));  
    write(su_shell_client_fd, "\n", 1);
  }
  else {
    su_srv_log_printf("Not connected, command canceled: %s\n", cmd_str);
    return -1;
  }
}

// Shared library initialization
//
void __attribute__ ((constructor)) libsusrv_init(void);
void libsusrv_init(void){}

// Shared library cleanup
//
void __attribute__ ((destructor)) libsusrv_exit(void);
void libsusrv_exit(void){
  //su_srv_close();
}

//
// su_srv_close(): Releases resources and close session
//
void su_srv_close(){
  //cleanup_rdv();
  
  if (su_shell_client_fd) {    
    su_srv_exe_nowait("exit");
    //close(su_shell_client_fd);
    su_shell_client_fd = 0;
  }

  su_srv_log_close();
}

