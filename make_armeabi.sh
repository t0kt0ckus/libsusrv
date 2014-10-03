#!/bin/sh

MODULE_NAME='susrv'
JMODULE_INSTALL_PATH='/opt/marl/research/android-studio-workspace/TalkToRil/app/libs'

echo '== Building ARMEABI library'
mkdir -p dist/lib/armeabi

cd jni
ndk-build clean
ndk-build
cd ..

cp libs/armeabi/lib${MODULE_NAME}.so dist/lib/armeabi
cd dist
zip -R lib${MODULE_NAME}.jar lib/armeabi/libsusrv.so
cd ..

