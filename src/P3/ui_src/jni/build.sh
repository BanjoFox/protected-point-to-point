#!/bin/sh
#
# Build the Java API to the P3 administration library
#

javac -classpath ./ *.java
export LD_LIBRARY_PATH=../.libs/
javah -classpath ./ -jni p3priAPI
javah -classpath ./ -jni p3secAPI

