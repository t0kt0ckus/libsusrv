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

// org.openmarl.susrv.SuShell.initSuSrv(): initializes a new SU shell session
//
// jAppDir: Android application private filesystem root
//
// Returns: 0 on success, negative value on error 
//
JNIEXPORT jint JNICALL
    Java_org_openmarl_susrv_SuShell_initSuSrv(JNIEnv * jEnv, jobject jInstance, jstring jAppDir);

// org.openmarl.susrv.SuShell.exec():
//
// jCommand: the shel command string
//
// Returns: the command result code or SU_SRV_CMD_FAILED (-999) when an error occured
//
JNIEXPORT jint JNICALL
    Java_org_openmarl_susrv_SuShell_exec(JNIEnv * jEnv, jobject jInstance, jstring jCommand);

// org.openmarl.susrv.SuShell.exitSuSrv(): exits current SU shell session
//
//
JNIEXPORT void JNICALL Java_org_openmarl_susrv_SuShell_exitSuSrv(JNIEnv * jEnv, jobject jInstance);
  
#endif
