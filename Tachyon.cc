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

jTachyonClient TachyonClient::createClient(const char *masterAddr)
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

  exception = callMethod(env, &ret, NULL, TFS_CLS, TFS_GET_METHD, 
                "(Ljava/lang/String;)Ltachyon/client/TachyonFS;", true, jPathStr);
  if (exception == NULL) {
    tfs = env->NewGlobalRef(ret.l);
    env->DeleteLocalRef(ret.l);
  } else {
    serror("fail to call TachyonFS.get()");
    printException(env, exception);
    tfs = NULL;
  }
  env->DeleteLocalRef(jPathStr); 
  return new TachyonClient(tfs);
}

jTachyonFile TachyonClient::getFile(const char * path)
{
  JNIEnv *env = getJNIEnv();
  if (env == NULL) {
    return NULL;
  }

  jthrowable exception;
  jvalue ret;
  jobject tfile;
  
  jstring jPathStr = env->NewStringUTF(path);
  if (jPathStr == NULL) {
    serror("fail to allocate path string");
    return NULL;
  }

  exception = callMethod(env, &ret, (jobject) m_tfs, TFS_CLS, TFS_GET_FILE_METHD, 
                "(Ljava/lang/String;)Ltachyon/client/TachyonFile;", false, jPathStr);
  if (exception == NULL) {
    tfile = env->NewGlobalRef(ret.l);
    env->DeleteLocalRef(ret.l);
  } else {
    serror("fail to call TachyonFS.getFile()");
    printException(env, exception);
    tfile = NULL;
  }
  env->DeleteLocalRef(jPathStr); 
  return new TachyonFile(tfile);
}

int TachyonClient::createFile(const char * path)
{
  return 0;
}

long TachyonFile::length()
{
  JNIEnv *env = getJNIEnv();
  if (env == NULL) {
    return NULL;
  }

  jthrowable exception;
  jvalue ret;
  
  exception = callMethod(env, &ret, (jobject) m_tfile, TFILE_CLS, TFILE_LENGTH_METHD, 
                "()J", false);
  if (exception == NULL) {
    return ret.j;
  } else {
    serror("fail to call TachyonFile.length()");
    return -1;
  }
}

/* vim: set ts=4 sw=4 : */
