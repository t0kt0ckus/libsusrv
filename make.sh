#!/bin/sh
#
# libsusrv: helper script to build distributable archive
#
# Warning: ANDROID_SDK environment variable must be set properly
#

# Android SDK configuration
#
ANDROID_SDK_LEVEL=19
# 
ANDROID_JAR=$ANDROID_SDK/platforms/android-$ANDROID_SDK_LEVEL/android.jar

# Directories setup
#
M_ROOT_DIR=`pwd`
TMP_DIR=$M_ROOT_DIR/tmp
rm -rf $TMP_DIR
mkdir -p $TMP_DIR
DIST_DIR=$M_ROOT_DIR/dist
rm -rf $DIST_DIR
mkdir -p $DIST_DIR
DIST_JAVADOC=$DIST_DIR/api
mkdir -p $DIST_JAVADOC
DIST_INCLUDE=$DIST_DIR/include
mkdir -p $DIST_INCLUDE

# Module definition
#
M_NAME="susrv"
M_NATIVE_DIR="$M_ROOT_DIR/jni"
M_JAVA_DIR="$M_ROOT_DIR/src"
M_JAVA_PKG="org.openmarl.susrv"
M_JAR="$DIST_DIR/lib$M_NAME.jar"

# libsusrv.so native libraries
#
cd $M_NATIVE_DIR
ndk-build
cd $M_ROOT_DIR
cp -R libs $TMP_DIR/lib

# org.openmarl.susrv Java package
#
javac -d $TMP_DIR -classpath $ANDROID_JAR $M_JAVA_DIR/*/*/*/*.java

# libsusrv.jar
#
cd $TMP_DIR
zip -R ${M_JAR} lib/*/*.so */*/*/*.class
cd $M_ROOT_DIR
rm -rf $TMP_DIR

# doc
#
javadoc -d $DIST_JAVADOC -quiet -windowtitle libsusrv -public -author -sourcepath $M_JAVA_DIR -classpath $ANDROID_JAR $M_JAVA_PKG
cp $M_NATIVE_DIR/su_srv.h $DIST_INCLUDE
cp README.md $DIST_DIR

