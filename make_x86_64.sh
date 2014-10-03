#!/bin/sh


MODULE_NAME='susrv'
MODULE_OBJS='obj/local/x86_64'
MODULE_SO=dist/lib/x86_64/lib${MODULE_NAME}.so

JAVA_HOME='/opt/platform/java/jse7'

echo '== Building X86_64 library'

mkdir -p obj/local/x86_64
mkdir -p dist/lib/x86_64
rm -f ${MODULE_OBJS}/*.o
rm -f ${MODULE_SO}

gcc -Wall -fPIC -c -o ${MODULE_OBJS}/su_srv_log.o jni/su_srv_log.c
gcc -Wall -fPIC -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux -c -o ${MODULE_OBJS}/su_srv.o jni/su_srv.c
gcc -Wall -fPIC -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux -c -o ${MODULE_OBJS}/su_srv_jni.o jni/su_srv_jni.c
gcc -shared -o ${MODULE_SO} ${MODULE_OBJS}/*.o


