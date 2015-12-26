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
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <string>
#include <stdexcept>
#include <map>

#define CTORNAME "<init>"

#define JTHROWABLE_CLS "java/lang/Throwable"
#define JRUNTIMEEXCEPT_CLS "RuntimeException"

#define JRUNTIMEEXCEPT_CTOR "(java/lang/String;)V"

#define JVM_BUF_LEN   1 
#define MAX_CLS_SIG 256
#define MAX_EXCEPT_MSG_LEN 256

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

namespace Tachyon { namespace JNI {

/**
 * simple pthread_mutex wrapper
 */
class Mutex { 

public:
  Mutex()
  {
    pthread_mutex_init(&m_mutex, NULL);
  }

  ~Mutex()
  {
    pthread_mutex_destroy(&m_mutex);
  }

  int lock()
  {
    return pthread_mutex_lock(&m_mutex);
  }

  int unlock()
  {
    return pthread_mutex_unlock(&m_mutex);
  }

private:
  pthread_mutex_t m_mutex;
};

/**
 * A wrapper for jthrowable into a C++ exception
 */
class JavaThrowableException {

public:
  JavaThrowableException(JNIEnv* env, jthrowable except)
  {
    m_env = env;
    m_except = (jthrowable) env->NewGlobalRef(except);
  }

  JavaThrowableException(JNIEnv* env)
  {
    m_env = env;
    jthrowable except = env->ExceptionOccurred();
    env->ExceptionClear();
    m_except = (jthrowable) env->NewGlobalRef(except);
  }

  ~JavaThrowableException()
  {
    m_env->DeleteGlobalRef(m_except);
  }

  jthrowable getException()
  {
    return (jthrowable) m_env->NewLocalRef(m_except);
  }

private:
  JNIEnv* m_env;
  jthrowable m_except;
};

/**
 * Exception that would be thrown while setting up JNI calls,
 * e.g., cannot find a class definition.
 *
 */
class NativeException: public std::exception {
public:
  NativeException() {}
  NativeException(const char* msg): m_msg(msg) {}

  virtual const char* what() throw() { return m_msg.c_str(); }

  ~NativeException() throw() {}

protected:
  std::string m_msg;
};

class ClassNotFoundException: public NativeException {
public:
  ClassNotFoundException(const char* className); 
};

class MethodNotFoundException: public NativeException {
public:
  MethodNotFoundException(const char* className, const char* methodName); 
};

class FieldNotFoundException: public NativeException {
public:
  FieldNotFoundException(const char* className, const char* fieldName); 
};

class NewGlobalRefException: public NativeException {
public:
  NewGlobalRefException(const char* refName); 
};

class NewObjectException: public NativeException {
public:
  NewObjectException(const char* className); 
};

class NewEnumException: public NativeException {
public:
  NewEnumException(const char* className, const char * valueName); 
};

class Env;
class JNIHelper;

class GlobalClassCache {
public: 
  GlobalClassCache() {}
  jclass get(const char *, Env* env); 
  bool set(const char *, jclass);

private:
  std::map<const char *, void *>  m_cls_map;
  Mutex m_lock;
};

class Env {

public:
  explicit Env();
  explicit Env(JNIEnv* env): m_env(env) {}

  Env(Env const & copy): m_env(copy.m_env) {}

  JNIEnv* get() { return m_env; }

  // simple FindClass wrapper with exception checking
  jclass findClass(const char *className);

  // similar to findClass except that the result would be cached
  // inside GlobalClassCache
  jclass findClassAndCache(const char *className);

  jobject newGlobalRef(jobject obj);
  void deleteLocalRef(jobject obj);

  // get method id in a java class
  jmethodID getMethodId(const char *className, const char *methodName, 
                        const char *methodSignature, bool isStatic);

  // create a new object for a class
  jobject newClassObject(const char *className, const char *ctorSignature, ...);

  // get the jobject for a enum value
  jobject getEnumObject(const char *className, const char * valueName);

  // create a runtime exception
  jthrowable newRuntimeException(const char *message);

  // get a method's return type based on the signature
  bool getMethodRetType(char * rettOut, const char *methodSignature);

  #define GET_JNI_X_FIELD(R, T)                                                   \
  R get##T##Field(jobject obj, jfieldID fid)                                      \
  {                                                                               \
    R ret = m_env->Get##T##Field(obj, fid);                                       \
    if (hasException()) {                                                         \
      m_env->ExceptionClear();                                                    \
      throw FieldNotFoundException("", "");                                       \
    }                                                                             \
    return ret;                                                                   \
  }

  GET_JNI_X_FIELD(jboolean, Boolean)
  GET_JNI_X_FIELD(jbyte, Byte)
  GET_JNI_X_FIELD(jchar, Char)
  GET_JNI_X_FIELD(jshort, Short)
  GET_JNI_X_FIELD(jint, Int)
  GET_JNI_X_FIELD(jlong, Long)
  GET_JNI_X_FIELD(jdouble, Double)
  GET_JNI_X_FIELD(jobject, Object)

  #undef GET_JNI_X_FIELD

  // invoke a method of a java class
  void callMethod(jvalue *retOut, jobject obj, const char *className, 
                const char *methodName, const char * methodSignature, bool isStatic, ...);

  // use macro concatenation to call the exact type of jni method and set
  // the return value (union) appropriately

  #define CALL_JNI_X_METHOD(R, T, F)                                            \
  inline void call##T##Method(jvalue *retOut,                                   \
          jobject obj, jmethodID mid, va_list args)                             \
  {                                                                             \
    R ret = m_env->Call##T##MethodV(obj, mid, args);                            \
    checkExceptionAndClear();                                                   \
    retOut->F = ret;                                                            \
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

  #define CALL_STATIC_JNI_X_METHOD(R, T, F)                                     \
  void callStatic##T##Method(jvalue *retOut,                                    \
                  jclass cls, jmethodID mid, va_list args)                      \
  {                                                                             \
    R ret = m_env->CallStatic##T##MethodV(cls, mid, args);                      \
    checkExceptionAndClear();                                                   \
    retOut->F = ret;                                                            \
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

  #undef CALL_STATIC_JNI_X_METHOD

  // void method is a bit different, define them separately
  inline void callVoidMethod(jobject obj, jmethodID mid, va_list args)
  {
    m_env->CallVoidMethodV(obj, mid, args);
    checkExceptionAndClear();
  }

  inline void callStaticVoidMethod(jclass cls, jmethodID mid, va_list args)
  {
    m_env->CallStaticVoidMethodV(cls, mid, args);
    checkExceptionAndClear();
  }

  bool hasException() {
    return m_env->ExceptionCheck();
  }

  void checkException();
  void checkExceptionAndClear();
  void checkExceptionAndAbort();
  void checkExceptionAndPrint();

  bool getClassName(jclass cls, jobject instance, std::string& nameStr);
  bool jstringToString(jstring str, std::string& cStr);
  bool throwableToString(jthrowable except, std::string& exceptStr);

private:

  JNIEnv* m_env;
};

/**
 * A singleton helper class to obtain the JNI env and facilitate other common
 * JNI routines 
 */
class JNIHelper {

public:
  static JNIHelper& get() {
    static JNIHelper instance;
    return instance;
  }
  ~JNIHelper() {}

  JNIEnv* getEnv();
  GlobalClassCache* getClassCache() { return &m_cls_cache; }

private:
  JNIHelper() {}
  JNIHelper(JNIHelper const &);
  void operator=(JNIHelper const &);

  Mutex m_env_lock;
  GlobalClassCache m_cls_cache;
}; // class JNIHelper

}} // namespace Tachyon::JNI

#endif /* __JNI_HELPER_H_ */

/* vim: set ts=4 sw=4 : */
