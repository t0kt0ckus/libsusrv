#include "su_srv.h"
#include "su_srv_jni.h"

JNIEXPORT jint JNICALL
  Java_org_openmarl_susrv_SuShell_open(JNIEnv * jEnv, jobject jInstance,
   jstring jRootFs)
{
  const char * rootfs_ptstr = (*jEnv)->GetStringUTFChars(jEnv,
    jRootFs,
    (jboolean *) 0);
  int result = su_srv_init(rootfs_ptstr);

  (*jEnv)-> ReleaseStringUTFChars(jEnv, jRootFs, rootfs_ptstr);
  return result;
}   


JNIEXPORT jint JNICALL
  Java_org_openmarl_susrv_SuShell_exec(JNIEnv * jEnv, jobject jInstance,
   jstring jCommand)
{
  const char * cmd_ptstr = (*jEnv)->GetStringUTFChars(jEnv,
    jCommand,
    (jboolean *) 0);

  int result = su_srv_exe(cmd_ptstr);

  (*jEnv)-> ReleaseStringUTFChars(jEnv, jCommand, cmd_ptstr);
  return result;
}

