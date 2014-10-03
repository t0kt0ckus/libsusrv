#!/bin/sh

cd jni
ndk-build clean
cd ..

rm -rf dist
rm -rf obj
rm -rf libs
 

