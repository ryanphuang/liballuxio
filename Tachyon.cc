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
  
  jstring jPathStr = env->NewStringUTF(masterUri);
  if (jPathStr == NULL) {
    serror("fail to allocate path string");
    return NULL;
  }

  exception = callMethod(env, &ret, NULL, TFS_CLS, TFS_GET_METHD, 
                "(Ljava/lang/String;)Ltachyon/client/TachyonFS;", true, jPathStr);
  env->DeleteLocalRef(jPathStr); 
  if (exception != NULL) {
    serror("fail to call TachyonFS.get()");
    printException(env, exception);
    return NULL;
  }
  return new TachyonClient(env, ret.l);
}

jTachyonFile TachyonClient::getFile(const char * path)
{
  jthrowable exception;
  jvalue ret;
  
  jstring jPathStr = m_env->NewStringUTF(path);
  if (jPathStr == NULL) {
    serror("fail to allocate path string");
    return NULL;
  }

  exception = callMethod(m_env, &ret, m_obj, TFS_CLS, TFS_GET_FILE_METHD, 
                "(Ljava/lang/String;)Ltachyon/client/TachyonFile;", false, jPathStr);
  m_env->DeleteLocalRef(jPathStr); 
  if (exception != NULL) {
    serror("fail to call TachyonFS.getFile()");
    printException(m_env, exception);
    return NULL;
  }
  return new TachyonFile(m_env, ret.l);
}

int TachyonClient::createFile(const char * path)
{
  return 0;
}

long TachyonFile::length()
{
  jthrowable exception;
  jvalue ret;
  
  exception = callMethod(m_env, &ret, m_obj, TFILE_CLS, TFILE_LENGTH_METHD, 
                "()J", false);
  if (exception != NULL) {
    serror("fail to call TachyonFile.length()");
    printException(m_env, exception);
    return -1;
  }
  return ret.j;
}

jTachyonByteBuffer TachyonFile::readByteBuffer(int blockIndex)
{
  jthrowable exception;
  jvalue ret;
  
  exception = callMethod(m_env, &ret, m_obj, TFILE_CLS, TFILE_RBB_METHD, 
                "(I)Ltachyon/client/TachyonByteBuffer;", false, (jint) blockIndex);
  if (exception != NULL) {
    serror("fail to call TachyonFile.getByteBuffer()");
    printException(m_env, exception);
    return NULL;
  }
  return new TachyonByteBuffer(m_env, ret.l);
}

jByteBuffer TachyonByteBuffer::getData()
{
  jthrowable exception;
  jobject ret;
  jclass cls;
  jfieldID fid;
  
  cls = m_env->GetObjectClass(m_obj);
  fid = m_env->GetFieldID(cls, "mData", "Ljava/nio/ByteBuffer;");
  if (fid == 0) {
    serror("fail to get field ID of TachyonByteBuffer.mData");
    return NULL;
  }
  ret = m_env->GetObjectField(m_obj, fid);
  exception = getAndClearException(m_env);
  if (exception != NULL) {
    printException(m_env, exception);
    return NULL;
  }
  return new ByteBuffer(m_env, ret);
}

void TachyonByteBuffer::close()
{

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
