/**
 *  @author        Ryan Huang <ryanhuang@cs.ucsd.edu>
 *  @organization  University of California, San Diego
 * 
 * JNI helper functions
 *
 */

#include "JNIHelper.h"
#include "Util.h"

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <sstream>

using namespace Tachyon::JNI;

std::map<JNIEnv*, ClassCache*> ClassCache::s_caches;

ClassNotFoundException::ClassNotFoundException(const char* className,
    JavaThrowable* detail)
{
  std::ostringstream ss;
  ss << "Could not find class " << className;
  m_msg = ss.str();
  m_detail = detail;
}

MethodNotFoundException::MethodNotFoundException(const char* className, 
    const char* methodName, JavaThrowable* detail)
{
  std::ostringstream ss;
  ss << "Could not find method " << methodName << " in class " << className; 
  m_msg = ss.str();
  m_detail = detail;
}

NewGlobalRefException::NewGlobalRefException(const char* refName,
    JavaThrowable* detail)
{
  std::ostringstream ss;
  ss << "Could not create global reference for " << refName;
  m_msg = ss.str();
  m_detail = detail;
}

NewObjectException::NewObjectException(const char* className,
    JavaThrowable* detail)
{
  std::ostringstream ss;
  ss << "Could not create object for class " << className;
  m_msg = ss.str();
  m_detail = detail;
}

NewEnumException::NewEnumException(const char* className, 
    const char * valueName, JavaThrowable* detail)
{
  std::ostringstream ss;
  ss << "Could not create enum for " << valueName << " of class " << className;
  m_msg = ss.str();
  m_detail = detail;
}

FieldNotFoundException::FieldNotFoundException(const char* className, 
    const char* fieldName, JavaThrowable* detail)
{
  std::ostringstream ss;
  ss << "Could not find field " << fieldName << " in class " << className;
  m_msg = ss.str();
  m_detail = detail;
}

ClassCache* ClassCache::instance(JNIEnv* env)
{
  std::map<JNIEnv*, ClassCache*>::iterator it = s_caches.find(env);
  if (it != s_caches.end()) {
    return it->second;
  }
  ClassCache* cache = new ClassCache(env);
  s_caches.insert(std::make_pair(env, cache));
  return cache;
}

jclass ClassCache::get(const char *className)
{
  m_lock.lock();
  std::map<const char *, jclass>::iterator it = m_cls_map.find(className);
  if (it != m_cls_map.end()) {
    // found in cache
    m_lock.unlock();
    return (jclass) it->second;
  }
  // cache miss, find the class and put into cache
  jclass globalCls = NULL;
  try {
    jclass cls = m_env.findClass(className);
    // find in env, first create a global reference (otherwise, the reference 
    // will be can be dangling), then update cache
    globalCls = (jclass) m_env.newGlobalRef(cls);
    m_env.deleteLocalRef(cls); // delete local reference
    m_cls_map.insert(std::make_pair(className, globalCls));
    m_lock.unlock();
    return globalCls;
  } catch (...) { 
    if (globalCls != NULL) {
      m_env.deleteGlobalRef(globalCls);
    }
    m_lock.unlock(); // unlock before re-throw
    throw;
  }
}

bool ClassCache::set(const char *className, jclass cls)
{
  m_lock.lock();
  m_cls_map.insert(std::make_pair(className, cls));
  m_lock.unlock();
  return true;
}

ClassCache::~ClassCache()
{
  std::map<const char *, jclass>::iterator it;
  for (it = m_cls_map.begin(); it != m_cls_map.end(); ++it) {
    jclass cls = (jclass) it->second;
    if (cls != NULL) {
      m_env->DeleteGlobalRef(cls);
    }
  }
}

Env::Env()
{
  m_env = JNIHelper::get().getEnv();
}

jclass Env::findClass(const char* className)
{
  jclass cls = m_env->FindClass(className);
  jthrowable except = m_env->ExceptionOccurred();
  if (except) {
    m_env->ExceptionClear();
    throw ClassNotFoundException(className, 
          new JavaThrowable(m_env, except));
  }
  return cls;
}

jclass Env::findClassAndCache(const char* className)
{
  return ClassCache::instance(m_env)->get(className);
}

jmethodID Env::getMethodId(const char *className, const char *methodName, 
                          const char *methodSignature, bool isStatic)
{
  jclass cls;
  jthrowable exception;
  jmethodID mid;

  try {
    cls = findClassAndCache(className);
    if (isStatic) {
      // find the static method
      mid = m_env->GetStaticMethodID(cls, methodName, methodSignature);
    } else {
      // find the instance method
      mid = m_env->GetMethodID(cls, methodName, methodSignature);
    }
    checkExceptionAndClear();
  } catch (const NativeException& exce) {
    // re-throw as MethodNotFoundException
    throw MethodNotFoundException(className, methodName, exce.detail());
  }
  if (mid == 0) {
    // 0 represents invalid method id
    throw MethodNotFoundException(className, methodName);
  }
  return mid;
}

jobject Env::newGlobalRef(jobject obj)
{
  jobject ret = m_env->NewGlobalRef(obj);
  if (m_env->ExceptionCheck()) {
    m_env->ExceptionClear();
    jclass cls = m_env->GetObjectClass(obj);
    std::string nameStr;
    if (getClassName(cls, obj, nameStr)) {
      throw NewGlobalRefException(nameStr.c_str());
    } else {
      throw NewGlobalRefException("unknown class");
    }
  }
  return ret;
}

void Env::deleteLocalRef(jobject obj)
{
  m_env->DeleteLocalRef(obj);
}

void Env::deleteGlobalRef(jobject obj)
{
  m_env->DeleteGlobalRef(obj);
}

bool Env::jstringToString(jstring str, std::string& cStr)
{
  if (str != NULL) {
    const char* buf = m_env->GetStringUTFChars(str, NULL);
    if (buf != NULL) {
      cStr += buf;
      m_env->ReleaseStringUTFChars(str, buf); 
    }
  }
  return true;
}

bool Env::getClassName(jclass cls, jobject instance, std::string& nameStr)
{
  jmethodID mid = m_env->GetMethodID(cls, "getClass", "()Ljava/lang/Class;");
  jobject clsObj = m_env->CallObjectMethod(instance, mid);
  // the getName method should be called on the base class instead of the
  // this object
  jclass baseCls = m_env->GetObjectClass(clsObj); // find the base class
  mid = m_env->GetMethodID(baseCls, "getName", "()Ljava/lang/String;");
  jstring clsName = (jstring) m_env->CallObjectMethod(clsObj, mid);
  return jstringToString(clsName, nameStr);
}

bool Env::throwableToString(jthrowable except, std::string& exceptStr)
{
  jclass cls = m_env->GetObjectClass(except);
  std::string nameStr;
  if (!getClassName(cls, except, nameStr)) {
    return false;
  }
  jmethodID mid = m_env->GetMethodID(cls, "getMessage", "()Ljava/lang/String;");
  jstring jmsg = (jstring) m_env->CallObjectMethod(except, mid);
  std::string msgStr;
  if (!jstringToString(jmsg, msgStr)) {
    return false;
  }
  exceptStr = nameStr + ": " + msgStr;
  return true;
}

bool Env::hasException() 
{
  return m_env->ExceptionCheck();
}

void Env::checkException()
{
  jthrowable except = m_env->ExceptionOccurred();
  if (except) {
    throw JavaThrowable(m_env, except);
  }
}

void Env::checkExceptionAndClear()
{
  jthrowable except = m_env->ExceptionOccurred();
  if (except) {
    // clear the exception in Java before throwing it into C++
    m_env->ExceptionClear();
    throw NativeException("Native exception", new JavaThrowable(m_env, except));
  }
}

void Env::checkExceptionAndAbort()
{
  jthrowable except = m_env->ExceptionOccurred();
  if (except) {
    // m_env->ExceptionDescribe();
    m_env->ExceptionClear();
    std::string exceptStr = "Abort due to JNI exception in ";
    throwableToString(except, exceptStr);
    throw std::runtime_error(exceptStr.c_str());
  }
}

void Env::checkExceptionAndPrint()
{
  jthrowable except = m_env->ExceptionOccurred();
  if (except) {
    m_env->ExceptionClear();
    std::string exceptStr;
    throwableToString(except, exceptStr);
    fprintf(stderr, "Exception occurred in Java %s\n", exceptStr.c_str());
  }
}

jobject Env::newClassObject(const char *className, const char *ctorSignature, ...)
{
  va_list args;
  jclass cls; // need class for new object
  jmethodID mid;
  jobject obj;

  try {
    cls = findClassAndCache(className);
    mid = getMethodId(className, CTORNAME, ctorSignature, false);
  } catch (const NativeException& exce) {
    throw NewObjectException(className, exce.detail());
  }

  va_start(args, ctorSignature);
  obj = m_env->NewObjectV(cls, mid, args);
  va_end(args);

  checkExceptionAndClear();
  return obj;
}

jobject Env::getEnumObject(const char *className, const char * valueName)
{
  jclass cls;
  jfieldID jid;
  jobject obj;

  try {
    cls = findClassAndCache(className);
    std::string clsSignature = className;
    clsSignature.insert(0, 1, 'L');
    clsSignature.push_back(';');
    jid = m_env->GetStaticFieldID(cls, valueName, clsSignature.c_str());
    obj = m_env->GetStaticObjectField(cls, jid);
    checkExceptionAndClear();
  } catch (const NativeException& exce) {
    throw NewEnumException(className, valueName, exce.detail());
  }

  return obj;
}

jthrowable Env::newRuntimeException(const char *message)
{
  jthrowable exception;
  jstring jmsg;
  jobject rteObj;

  jmsg = m_env->NewStringUTF(message);
  checkExceptionAndAbort();

  rteObj = newClassObject(JRUNTIMEEXCEPT_CLS, JRUNTIMEEXCEPT_CTOR, jmsg);
  m_env->DeleteLocalRef(jmsg);
  // TODO: potential mem leak or stale object reference
  return (jthrowable) rteObj;
}

bool Env::getMethodRetType(char * rettOut, const char *methodSignature)
{
  if (rettOut == NULL)
    return false;
  char t = findNext(methodSignature, ')');
  if (t == '\0') {
    // invalid signature
    return false;
  }
  *rettOut = t;
  return true;
}

void Env::callMethod(jvalue *retOut, jobject obj, const char *className, 
      const char *methodName, const char * methodSignature, bool isStatic, ...)
{
  va_list args;
  jclass cls;
  jmethodID mid;

  cls = findClassAndCache(className);
  mid = getMethodId(className, methodName, methodSignature, isStatic);

  try {
    va_start(args, isStatic);
    char retType;
    if (!getMethodRetType(&retType, methodSignature)) {
      std::ostringstream ss;
      ss << "Could not get method return type for " << 
                className << ":" << methodName;
      std::string msg = ss.str();
      throw NativeException(msg.c_str());
    }
    switch (retType) {
      case J_BOOL:
          if (isStatic)
            callStaticBooleanMethod(retOut, cls, mid, args);
          else
            callBooleanMethod(retOut, obj, mid, args);
          break;
      case J_BYTE:
          if (isStatic)
            callStaticByteMethod(retOut, cls, mid, args);
          else
            callByteMethod(retOut, obj, mid, args);
          break;
      case J_CHAR:
          if (isStatic)
            callStaticCharMethod(retOut, cls, mid, args);
          else
            callCharMethod(retOut, obj, mid, args);
          break;
      case J_SHORT:
          if (isStatic)
            callStaticShortMethod(retOut, cls, mid, args);
          else
            callShortMethod(retOut, obj, mid, args);
          break;
      case J_INT:
          if (isStatic)
            callStaticIntMethod(retOut, cls, mid, args);
          else
            callIntMethod(retOut, obj, mid, args);
          break;
      case J_LONG:
          if (isStatic)
            callStaticLongMethod(retOut, cls, mid, args);
          else
            callLongMethod(retOut, obj, mid, args);
          break;
      case J_FLOAT:
          if (isStatic)
            callStaticFloatMethod(retOut, cls, mid, args);
          else
            callFloatMethod(retOut, obj, mid, args);
          break;
      case J_DOUBLE:
          if (isStatic)
            callStaticDoubleMethod(retOut, cls, mid, args);
          else
            callDoubleMethod(retOut, obj, mid, args);
          break;
      case J_VOID: 
          if (isStatic)
            callStaticVoidMethod(cls, mid, args);
          else
            callVoidMethod(obj, mid, args);
          break;
      case J_OBJ: 
      case J_ARRAY: 
          if (isStatic)
            callStaticObjectMethod(retOut, cls, mid, args);
          else
            callObjectMethod(retOut, obj, mid, args);
          break;
      default: 
          {
            std::ostringstream ss;
            ss << "Unrecognized method return type for " << className << 
                      ":" << methodName;
            std::string msg = ss.str();
            throw NativeException(msg.c_str());
          }
    }
  } catch(...) {
    va_end(args);
    throw;
  }
  va_end(args);
}

JNIEnv* JNIHelper::getEnv()
{
  JNIEnv* env = NULL;
  JavaVM* vmBuf[JVM_BUF_LEN];
  jsize vms;

  m_env_lock.lock(); // ensure thread-safe JNIEnv acquisition

  jint ok = JNI_GetCreatedJavaVMs(vmBuf, JVM_BUF_LEN, &vms);
  if (ok != JNI_OK) {
    m_env_lock.unlock();
    throw std::runtime_error("JNI_GetCreatedJavaVMs call failed");
  }
  if (vms == 0) {
    // no JVM has been created yet, create now
    JavaVM* jvm;
    
    JavaVMOption options[1];
    char *classpath = getenv("CLASSPATH");
    if (classpath == NULL) {
      m_env_lock.unlock();
      throw std::runtime_error("CLASSPATH env variable is not set");
    }
    const char *classpath_opt = "-Djava.class.path=";
    size_t cp_len = strlen(classpath) + strlen(classpath_opt) + 1;
    char * classpath_arg = (char *) malloc(sizeof(char) * cp_len);
    snprintf(classpath_arg, cp_len, "%s%s", classpath_opt, classpath);
    options[0].optionString = classpath_arg;
    // TODO: we may want to add other JVM options

    JavaVMInitArgs args;
    args.version = JNI_VERSION_1_2;
    args.nOptions = 1;
    args.options = options;
    args.ignoreUnrecognized = JNI_TRUE;
    
    ok = JNI_CreateJavaVM(&jvm, (void **) &env, &args);
    if (ok != JNI_OK) {
      m_env_lock.unlock();
      throw std::runtime_error("JNI_CreateJavaVM call failed");
    }
  } else {
    // there are JVM created already, re-use
    JavaVM* jvm = vmBuf[0];
    ok = jvm->AttachCurrentThread((void **) &env, NULL);
    if (ok != JNI_OK) {
      m_env_lock.unlock();
      throw std::runtime_error("AttachCurrentThread call failed");
    }
  }

  m_env_lock.unlock(); // end of critical section
  return env;
}


/* vim: set ts=4 sw=4 : */
