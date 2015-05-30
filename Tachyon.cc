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

#include <string.h>
#include <stdlib.h>

jTachyonClient TachyonClient::createClient(const char *masterUri)
{
  JNIEnv *env = getJNIEnv();
  if (env == NULL) {
    return NULL;
  }

  jthrowable exception;
  jvalue ret;
  jobject tfs;
  
  jstring jPathStr = env->NewStringUTF(masterUri);
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

char* fullTachyonPath(const char *masterUri, const char *filePath)
{
  size_t mlen, flen, nlen;
  bool slash;
  char *fullPath;

  mlen = strlen(masterUri);
  flen = strlen(filePath);
  if (mlen == 0 || flen == 0) {
    return NULL;
  }
  if (flen >= mlen) {
    if (strncmp(masterUri, filePath, mlen) == 0) {
      // it's already a full path
      fullPath = (char *) malloc((flen + 1) * sizeof(char));
      if (fullPath != NULL) {
        strncpy(fullPath, filePath, flen);
        fullPath[flen] = '\0';
      }
      return fullPath;
    }
  }
  slash = (masterUri[mlen - 1] == '/' || filePath[flen - 1] != '/');
  if (slash)
    nlen = mlen + flen + 1;
  else
    nlen = mlen + flen + 2; // need to insert a slash
  fullPath = (char *) malloc(nlen * sizeof(char));
  if (fullPath != NULL) {
    if (slash)
      snprintf(fullPath, nlen, "%s%s", masterUri, filePath);
    else
      snprintf(fullPath, nlen, "%s/%s", masterUri, filePath);
  }
  return fullPath;
}

/* vim: set ts=4 sw=4 : */
