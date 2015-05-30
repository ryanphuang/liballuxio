/**
 *  @author        Ryan Huang <ryanhuang@cs.ucsd.edu>
 *  @organization  University of California, San Diego
 * 
 * JNI helper functions
 *
 */

#ifndef __JNI_HELPER_H__
#define __JNI_HELPER_H__

#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define CTORNAME "<init>"

#define JTHROWABLE_CLS "java/lang/Throwable"

// jvm type signature
// reference: http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/types.html#wp9502

#define J_BOOL   'Z'
#define J_BYTE   'B'
#define J_CHAR   'C'
#define J_SHORT  'S'
#define J_INT    'I'
#define J_LONG   'J'
#define J_FLOAT  'F'
#define J_DOUBLE 'D'
#define J_VOID   'V'
#define J_OBJ    'L'
#define J_ARRAY  '['

#ifdef __cplusplus
extern "C" {
#endif

// remember the JNIEnv so we don't need to create each time
extern JNIEnv* globalEnv; 

// create jvm and jni env
JNIEnv* createJNIEnv();

// get the current JNIEnv, create if necessary
JNIEnv* getJNIEnv();

// get a java class
jthrowable getClass(JNIEnv *env, jclass *clsOut, const char *className);

// get method id in a java class
jthrowable getMethodId(JNIEnv *env, jmethodID *methodIdOut, const char *className,
                const char *methodName, const char *methodSignature, bool isStatic);

// get the exception in env and clear it
jthrowable getAndClearException(JNIEnv *env);

// create a new object for a class
jthrowable newClassObject(JNIEnv *env, jobject *objOut, const char *className,
                const char *ctorSignature, ...);

// get a method's return type based on the signature
bool getMethodRetType(char * rettOut, const char *methodSignature);

// print java throwable exception
void printException(JNIEnv *env, jthrowable exception);

// invoke a method of a java class
jthrowable callMethod(JNIEnv *env, jvalue *retOut, jobject obj, 
                const char *className, const char *methodName, 
                const char * methodSignature, bool isStatic, ...);

// use macro concatenation to call the exact type of jni method and set
// the return value (union) appropriately

#define CALL_JNI_X_METHOD(R, T, F)                                             \
inline jthrowable call##T##Method(JNIEnv *env, jvalue *retOut, jobject obj,    \
                jmethodID mid, va_list args)                                   \
  {                                                                            \
    jthrowable exception;                                                      \
    R ret = env->Call##T##MethodV(obj, mid, args);                             \
    exception = getAndClearException(env);                                     \
    retOut->F = ret;                                                           \
    return exception;                                                          \
  } 
    
// refer to JNI specification for the encodings
CALL_JNI_X_METHOD(jboolean, Boolean, z)
CALL_JNI_X_METHOD(jbyte, Byte, b)
CALL_JNI_X_METHOD(jchar, Char, c)
CALL_JNI_X_METHOD(jshort, Short, s)
CALL_JNI_X_METHOD(jint, Int, i)
CALL_JNI_X_METHOD(jlong, Long, j)
CALL_JNI_X_METHOD(jfloat, Float, f)
CALL_JNI_X_METHOD(jdouble, Double, d)
CALL_JNI_X_METHOD(jobject, Object, l)

#undef CALL_JNI_X_METHOD

#define CALL_STATIC_JNI_X_METHOD(R, T, F)                                       \
inline  jthrowable callStatic##T##Method(JNIEnv *env, jvalue *retOut,           \
                  jclass cls, jmethodID mid, va_list args)                      \
  {                                                                             \
    jthrowable exception;                                                       \
    R ret = env->CallStatic##T##MethodV(cls, mid, args);                        \
    exception = getAndClearException(env);                                      \
    retOut->F = ret;                                                            \
    return exception;                                                           \
  }

// refer to JNI specification for the encodings
CALL_STATIC_JNI_X_METHOD(jboolean, Boolean, z)
CALL_STATIC_JNI_X_METHOD(jbyte, Byte, b)
CALL_STATIC_JNI_X_METHOD(jchar, Char, c)
CALL_STATIC_JNI_X_METHOD(jshort, Short, s)
CALL_STATIC_JNI_X_METHOD(jint, Int, i)
CALL_STATIC_JNI_X_METHOD(jlong, Long, j)
CALL_STATIC_JNI_X_METHOD(jfloat, Float, f)
CALL_STATIC_JNI_X_METHOD(jdouble, Double, d)
CALL_STATIC_JNI_X_METHOD(jobject, Object, l)

// void method is a bit different, define them separately
inline jthrowable callVoidMethod(JNIEnv *env, jobject obj, jmethodID mid, 
                  va_list args)
{
  jthrowable exception;
  env->CallVoidMethodV(obj, mid, args);
  exception = getAndClearException(env);
  return exception;
}

inline jthrowable callStaticVoidMethod(JNIEnv *env, jclass cls, jmethodID mid, 
                  va_list args)
{
  jthrowable exception;
  env->CallStaticVoidMethodV(cls, mid, args);
  exception = getAndClearException(env);
  return exception;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __JNI_HELPER_H_ */

/* vim: set ts=4 sw=4 : */
