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
#include "su_srv_jni.h"

JNIEXPORT jint JNICALL
    Java_org_openmarl_susrv_SuShell_initSuSrv(JNIEnv * jEnv, jobject jInstance, jstring jRootFs)
{
    const char * rootfs_ptstr = (*jEnv)->GetStringUTFChars(jEnv, jRootFs, (jboolean *) 0);
    int result = su_srv_init(rootfs_ptstr);
    (*jEnv)-> ReleaseStringUTFChars(jEnv, jRootFs, rootfs_ptstr);
    return result;
}   


JNIEXPORT jint JNICALL
    Java_org_openmarl_susrv_SuShell_exec(JNIEnv * jEnv, jobject jInstance, jstring jCommand)
{
    const char * cmd_ptstr = (*jEnv)->GetStringUTFChars(jEnv, jCommand, (jboolean *) 0);
    int result = su_srv_exe(cmd_ptstr);
    (*jEnv)-> ReleaseStringUTFChars(jEnv, jCommand, cmd_ptstr);
    return result;
}

JNIEXPORT void JNICALL Java_org_openmarl_susrv_SuShell_exitSuSrv(JNIEnv * jEnv, jobject jInstance)
{
    su_srv_exit();
}
  
