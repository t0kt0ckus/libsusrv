/*
 * libsusrv: Android SU native client library.
 *
 * <t0kt0ckus@gmail.com>
 * (C) 2014
 *
 * License: GPLv3
 *
 */
#ifndef __SU_SRV_JNI_H
#define __SU_SRV_JNI_H

#include <jni.h>

JNIEXPORT jint JNICALL
    Java_org_openmarl_susrv_LibSusrv_openShellSession(JNIEnv *jEnv, 
            jobject jInstance,
            jstring jPfsRoot);

JNIEXPORT jint JNICALL
    Java_org_openmarl_susrv_LibSusrv_exec(JNIEnv *jEnv, jobject jInstance, jstring jCommand);

JNIEXPORT jint JNICALL
    Java_org_openmarl_susrv_LibSusrv_exitShellSession(JNIEnv *jEnv, jobject jInstance);
  
JNIEXPORT jint JNICALL
    Java_org_openmarl_susrv_LibSusrv_ping(JNIEnv *jEnv, jobject jInstance);

JNIEXPORT jint JNICALL
    Java_org_openmarl_susrv_LibSusrv_getpid(JNIEnv *jEnv, jobject jInstance, 
    jstring jProcname);

#endif
