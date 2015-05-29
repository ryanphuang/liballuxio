#ifndef __JNI_HELPER_H_
#define __JNI_HELPER_H_

#include <jni.h>
#include <stdlib.h>
#include <stdio.h>

// remember the JNIEnv so we don't need to create each time
extern JNIEnv* globalEnv; 

// create jvm and jni env
JNIEnv* createJNIEnv();

// get the current JNIEnv, create if necessary
JNIEnv* getJNIEnv();

// find a java class
jthrowable findClass(JNIEnv *env, const char *className, jclass *cls);

// invoke a method of a java class
jthrowable callMethod(JNIEnv *env, jobject obj);


#endif /* __JNI_HELPER_H_ */

/* vim: set ts=4 sw=4 : */
