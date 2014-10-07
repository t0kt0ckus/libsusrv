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
#ifndef __SU_SRV_JNI_H
#define __SU_SRV_JNI_H

#include <jni.h>

JNIEXPORT jint JNICALL
  Java_org_openmarl_susrv_SuShell_open(JNIEnv * jEnv, jobject jInstance,
   jstring jRootFs);

JNIEXPORT jint JNICALL
  Java_org_openmarl_susrv_SuShell_exec(JNIEnv * jEnv, jobject jInstance,
   jstring jCommand);

JNIEXPORT void JNICALL
  Java_org_openmarl_susrv_SuShell_close(JNIEnv * jEnv, jobject jInstance);
  
#endif
