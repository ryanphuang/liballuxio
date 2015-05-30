/**
 *  @author        Ryan Huang <ryanhuang@cs.ucsd.edu>
 *  @organization  University of California, San Diego
 * 
 * Tachyon C/C++ APIs
 *
 */

#include "Tachyon.h"
#include "Util.h"
#include "JNIHelper.h"

TachyonClient createClient(const char *masterAddr)
{
  JNIEnv *env = getJNIEnv();
  if (env == NULL) {
    return NULL;
  }

  jthrowable exception;
  jvalue ret;
  jobject tfs;
  
  jstring jPathStr = env->NewStringUTF(masterAddr);
  if (jPathStr == NULL) {
    serror("fail to allocate path string");
    return NULL;
  }

  exception = callMethod(env, &ret, NULL, TFS_CLS, TFS_GET_NAME, 
                "(Ljava/lang/String;)Ltachyon/client/TachyonFS;", true, jPathStr);
  if (exception == NULL) {
    tfs = env->NewGlobalRef(ret.l);
    env->DeleteLocalRef(ret.l);
  } else {
    serror("fail to call TachyonFS get");
    printException(env, exception);
    tfs = NULL;
  }
  env->DeleteLocalRef(jPathStr); 
  return (TachyonClient) tfs;
}

int createFile(TachyonClient client, const char * path)
{

  return 0;
}

/* vim: set ts=4 sw=4 : */
