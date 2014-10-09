LOCAL_PATH := $(call my-dir)

# libsusrv: our SU shell provider library 
##
include $(CLEAR_VARS)
LOCAL_MODULE := susrv
LOCAL_SRC_FILES := su_srv.c su_srv_jni.c su_srv_log.c
LOCAL_CFLAGS := -g -Wall
include $(BUILD_SHARED_LIBRARY)

