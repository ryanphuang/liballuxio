#include "JNIHelper.h"
#include "Util.h"

#include <map>

JNIEnv* globalEnv = NULL;

typedef std::map<const char *, void *> HashMap;
typedef std::map<const char *, void *>::iterator KVPair;

HashMap globalClsMap;

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
  
  err = JNI_CreateJavaVM(&jvm, (void **) env, &args);

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

jthrowable findClass(JNIEnv *env, const char *className, jclass *cls)
{
  jthrowable exception = NULL;
  KVPair pair = globalClsMap.find(className);

  return exception;
}

jthrowable callMethod(JNIEnv *env, jobject obj)
{
  jthrowable exception = NULL;

  return exception;
}

/* vim: set ts=4 sw=4 : */
