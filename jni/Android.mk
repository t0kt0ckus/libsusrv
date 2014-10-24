LOCAL_PATH := $(call my-dir)

# libsusrv: our SU shell provider library 
##
include $(CLEAR_VARS)
LOCAL_MODULE := susrv
LOCAL_SRC_FILES := su_srv.c su_srv_jni.c su_srv_log.c su_srv_pfs.c su_shell_session.c
LOCAL_CFLAGS := -g -Wall
include $(BUILD_SHARED_LIBRARY)

# sutest: a simple test client 
##
include $(CLEAR_VARS)
LOCAL_MODULE := sutest
LOCAL_SRC_FILES := main.c
LOCAL_CFLAGS := -g -Wall 
LOCAL_SHARED_LIBRARIES := susrv 
include $(BUILD_EXECUTABLE)

