/*
 * libsusrv: Android SU native client library.
 *
 * <t0kt0ckus@gmail.com>
 * (C) 2014
 *
 * License: GPLv3
 *
 */
#include "su_srv.h"
#include "su_srv_getpid.h"
#include "su_srv_jni.h"

JNIEXPORT jint JNICALL
    Java_org_openmarl_susrv_LibSusrv_openShellSession(JNIEnv *jEnv,
            jobject jInstance,
            jstring jPfsRoot)
{
    const char *pfs_root = (*jEnv)->GetStringUTFChars(jEnv, jPfsRoot, (jboolean *) 0);
    int result = su_srv_open_shell_session(pfs_root);
    (*jEnv)-> ReleaseStringUTFChars(jEnv, jPfsRoot, pfs_root);
    return result;
}   


JNIEXPORT jint JNICALL
    Java_org_openmarl_susrv_LibSusrv_exec(JNIEnv * jEnv, jobject jInstance, jstring jCommand)
{
    const char * cmd_ptstr = (*jEnv)->GetStringUTFChars(jEnv, jCommand, (jboolean *) 0);
    int result = su_srv_exec(cmd_ptstr);
    (*jEnv)-> ReleaseStringUTFChars(jEnv, jCommand, cmd_ptstr);
    return result;
}

JNIEXPORT jstring JNICALL
    Java_org_openmarl_susrv_LibSusrv_getLastTtyRead(JNIEnv *jEnv,
            jobject jInstance)
{
    char *line = su_srv_last_tty_read();
    if (line)
        return (*jEnv)->NewStringUTF(jEnv, line);
    else 
        return (*jEnv)->NewStringUTF(jEnv, "");
}
    
JNIEXPORT jint JNICALL
    Java_org_openmarl_susrv_LibSusrv_getpid(JNIEnv *jEnv, jobject jInstance, 
    jstring jProcname)
{
    const char * procname_ptstr = (*jEnv)->GetStringUTFChars(jEnv, jProcname, (jboolean *) 0);
    int result = su_srv_getpid(procname_ptstr);
    (*jEnv)-> ReleaseStringUTFChars(jEnv, jProcname, procname_ptstr);
    return result;
}

JNIEXPORT jint JNICALL Java_org_openmarl_susrv_LibSusrv_exitShellSession(JNIEnv * jEnv,
        jobject jInstance)
{
    return su_srv_exit_shell_session();
}
  
JNIEXPORT jint JNICALL
    Java_org_openmarl_susrv_LibSusrv_ping(JNIEnv *jEnv, jobject jInstance)
{
    return su_srv_shell_session_ping();
}

JNIEXPORT void JNICALL
    Java_org_openmarl_susrv_LibSusrv_setTtyEcho(JNIEnv *jEnv, 
            jobject jInstance,
            jint jEcho)
{
    su_srv_set_tty_echo(jEcho);
}

JNIEXPORT jint JNICALL
    Java_org_openmarl_susrv_LibSusrv_getTtyEcho(JNIEnv *jEnv, 
            jobject jInstance)
{
    return su_srv_get_tty_echo();
}
