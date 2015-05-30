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
#include <map>

JNIEnv* globalEnv = NULL;

typedef std::map<const char *, void *> HashMap;
typedef std::map<const char *, void *>::iterator MapIter;
typedef std::pair<const char *, void *> KVPair;

HashMap globalClsMap;

void * mapGet(HashMap& map, const char * key)
{
  MapIter it = map.find(key);
  if (it == map.end()) {
    return NULL;
  }
  return it->second;
}

void mapPut(HashMap& map, const char * key, void * value)
{
  map.insert(KVPair(key, value));
}

// Create a JNI env. 
//
// Reference: http://www.inonit.com/cygwin/jni/invocationApi/c.html
JNIEnv* createJNIEnv() 
{
  JavaVM* jvm;
  JNIEnv* env;
  
  JavaVMInitArgs args;
  JavaVMOption options[1];

  const char *classpath_prefix = "-Djava.class.path=";
  char *classpath;
  char *classpath_arg;

  jint err = 0;

  args.version = JNI_VERSION_1_2;
  args.nOptions = 1;
  classpath = getenv("CLASSPATH"); // read CLASSPATH env variable
  if (classpath == NULL) {
    die("CLASSPATH env variable is not set!");
  }
  classpath_arg = concat(classpath_prefix, classpath);
  if (classpath_arg == NULL) {
    die("fail to allocate classpath arg string");
  }
  options[0].optionString = classpath_arg;
  args.options = options;
  
  err = JNI_CreateJavaVM(&jvm, (void **) &env, &args);

  // before checking return value, we should free the allocated string
  free(classpath_arg);

  if (err != 0) {
    die("fail to create JVM");
  }
  return env;
}

// TODO: not thread-safe
JNIEnv* getJNIEnv()
{
  if (globalEnv == NULL) {
    globalEnv = createJNIEnv();
  }
  return globalEnv;
}

jthrowable getAndClearException(JNIEnv *env)
{
  jthrowable exception = env->ExceptionOccurred();
  if (!exception)
    return NULL;
  env->ExceptionClear();
  return exception;
}

jthrowable getMethodId(JNIEnv *env, jmethodID *methodIdOut, const char *className,
                const char *methodName, const char *methodSignature, bool isStatic)
{
  jclass cls;
  jthrowable exception;
  jmethodID mid;

  exception = getClass(env, &cls, className);
  if (exception != NULL)
    return exception; 
  if (isStatic) {
    // find the static method
    mid = env->GetStaticMethodID(cls, methodName, methodSignature);
  } else {
    // find the instance method
    mid = env->GetMethodID(cls, methodName, methodSignature);
  }
  if (mid == 0) {
    exception = getAndClearException(env);
  } else {
    *methodIdOut = mid;
    exception = NULL;
  }
  return exception;
}

jthrowable getClass(JNIEnv *env, jclass *clsOut, const char *className)
{
  jthrowable exception = NULL;
  jclass cls = NULL;

  cls = (jclass) mapGet(globalClsMap, className);
  if (cls != NULL) { // found in cache
    *clsOut = cls;
    return exception;
  } else {
    // not in cache, find from env
    cls = env->FindClass(className);
    if (cls == NULL) {
      // still can't find from env
      exception = getAndClearException(env);
    } else {
      // find in env, first create a global reference (otherwise, the reference will be
      // can be dangling), then update cache
      jclass globalCls = (jclass) env->NewGlobalRef(cls);
      if (globalCls == NULL) {
        exception = getAndClearException(env);
      } else {
        mapPut(globalClsMap, className, globalCls);
        *clsOut = globalCls;
        env->DeleteLocalRef(cls); // delete local reference
      }
    }
  }
  return exception;
}

jthrowable newClassObject(JNIEnv *env, jobject *objOut, const char *className,
              const char *ctorSignature, ...)
{
  va_list args;
  jthrowable exception;
  jclass cls; // need class for new object
  jmethodID mid;
  jobject obj;

  exception = getClass(env, &cls, className);
  if (exception != NULL) {
    return exception;
  }

  exception = getMethodId(env, &mid, className, CTORNAME, ctorSignature, false);
  if (exception != NULL) {
    return exception;
  }

  va_start(args, ctorSignature);
  obj = env->NewObjectV(cls, mid, args);
  va_end(args);

  if (obj == NULL) {
    exception = getAndClearException(env);
  } else {
    *objOut = obj;
    exception = NULL;
  }
  return exception;
}

bool getMethodRetType(char * rettOut, const char *methodSignature)
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

void printException(JNIEnv *env, jthrowable exception)
{
  jclass cls;
  jclass basecls;
  jobject obj;
  jmethodID mid;

  jstring jClsName;
  jstring jMsg;

  cls = env->GetObjectClass(exception);
  mid = env->GetMethodID(cls, "getClass", "()Ljava/lang/Class;");
  obj = env->CallObjectMethod(exception, mid);
  basecls = env->GetObjectClass(obj);
  mid = env->GetMethodID(basecls, "getName", "()Ljava/lang/String;");
  jClsName = (jstring) env->CallObjectMethod(obj, mid);

  mid = env->GetMethodID(cls, "getMessage", "()Ljava/lang/String;");
  jMsg = (jstring) env->CallObjectMethod(exception, mid);

  const char *clsName = env->GetStringUTFChars(jClsName, 0);
  const char *msg = env->GetStringUTFChars(jMsg, 0); 

  printf("exception in %s: %s\n", clsName, msg);
  env->ReleaseStringUTFChars(jClsName, clsName); 
  env->ReleaseStringUTFChars(jMsg, msg); 
}

jthrowable callMethod(JNIEnv *env, jvalue *retOut, jobject obj, 
                const char *className, const char *methodName, 
                const char * methodSignature, bool isStatic, ...)
{
  va_list args;
  jthrowable exception;
  jclass cls;
  jmethodID mid;

  exception = getClass(env, &cls, className);
  if (exception != NULL)
    return exception;

  exception = getMethodId(env, &mid, className, methodName, 
                          methodSignature, isStatic);
  if (exception != NULL)
    return exception;

  va_start(args, isStatic);
  char retType;
  getMethodRetType(&retType, methodSignature);
  switch (retType) {
    case J_BOOL:
        if (isStatic)
          exception = callStaticBooleanMethod(env, retOut, cls, mid, args);
        else
          exception = callBooleanMethod(env, retOut, obj, mid, args);
        break;
    case J_BYTE:
        if (isStatic)
          exception = callStaticByteMethod(env, retOut, cls, mid, args);
        else
          exception = callByteMethod(env, retOut, obj, mid, args);
        break;
    case J_CHAR:
        if (isStatic)
          exception = callStaticCharMethod(env, retOut, cls, mid, args);
        else
          exception = callCharMethod(env, retOut, obj, mid, args);
        break;
    case J_SHORT:
        if (isStatic)
          exception = callStaticShortMethod(env, retOut, cls, mid, args);
        else
          exception = callShortMethod(env, retOut, obj, mid, args);
        break;
    case J_INT:
        if (isStatic)
          exception = callStaticIntMethod(env, retOut, cls, mid, args);
        else
          exception = callIntMethod(env, retOut, obj, mid, args);
        break;
    case J_LONG:
        if (isStatic)
          exception = callStaticLongMethod(env, retOut, cls, mid, args);
        else
          exception = callLongMethod(env, retOut, obj, mid, args);
        break;
    case J_FLOAT:
        if (isStatic)
          exception = callStaticFloatMethod(env, retOut, cls, mid, args);
        else
          exception = callFloatMethod(env, retOut, obj, mid, args);
        break;
    case J_DOUBLE:
        if (isStatic)
          exception = callStaticDoubleMethod(env, retOut, cls, mid, args);
        else
          exception = callDoubleMethod(env, retOut, obj, mid, args);
        break;
    case J_VOID: 
        if (isStatic)
          exception = callStaticVoidMethod(env, cls, mid, args);
        else
          exception = callVoidMethod(env, obj, mid, args);
        break;
    case J_OBJ: 
    case J_ARRAY: 
        if (isStatic)
          exception = callStaticObjectMethod(env, retOut, cls, mid, args);
        else
          exception = callObjectMethod(env, retOut, obj, mid, args);
        break;
    default: 
        // TODO: throw a new exception to handle unrecognized return type
        break;
  }
  va_end(args);
  return exception;
}


/* vim: set ts=4 sw=4 : */
