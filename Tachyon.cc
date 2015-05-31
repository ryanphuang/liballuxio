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

using namespace tachyon;

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

jInStream TachyonFile::getInStream(ReadType readType)
{
  jthrowable exception;
  jvalue ret;
  jobject eobj;

  exception = enumObjReadType(m_env, &eobj, readType);
  if (exception != NULL) {
    serror("fail to get enum obj for read type");
    printException(m_env, exception);
    return NULL;
  }
  
  exception = callMethod(m_env, &ret, m_obj, TFILE_CLS, TFILE_GIS_METHD, 
                "(Ltachyon/client/ReadType;)Ltachyon/client/InStream;", false, eobj);
  if (exception != NULL) {
    serror("fail to call TachyonFile.getInStream()");
    printException(m_env, exception);
    return NULL;
  }
  return new InStream(m_env, ret.l);
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
  callMethod(m_env, NULL, m_obj, TBBUF_CLS, TBBUF_CLOSE_METHD, "()V", false);
}

jByteBuffer ByteBuffer::allocate(int capacity)
{
  jthrowable exception;
  jvalue ret;

  JNIEnv *env = getJNIEnv();
  if (env == NULL) {
    return NULL;
  }

  exception = callMethod(env, &ret, NULL, BBUF_CLS, BBUF_ALLOC_METHD, 
                "(I)Ljava/nio/ByteBuffer;", true, (jint) capacity);

  if (exception != NULL) {
    serror("fail to call ByteBuffer.allocate()");
    printException(env, exception);
    return NULL;
  }
  return new ByteBuffer(env, ret.l);
}

int InStream::read() 
{
  jthrowable exception;
  jvalue ret;

  exception = callMethod(m_env, &ret, m_obj, TINSTREAM_CLS, TINSTREAM_READ_METHD,
                "()I", false);
  if (exception != NULL) {
    serror("fail to call InStream.Read()");
    printException(m_env, exception);
    return NULL;
  }
  return ret.i;
}

int InStream::read(void *buff, int length)
{
  return read(buff, length, 0, length);
}

int InStream::read(void *buff, int length, int off, int maxLen)
{
  jthrowable exception;
  jbyteArray jBuf;
  jvalue ret;
  int rdSz;

  jBuf = m_env->NewByteArray(length);
  if (jBuf == NULL) {
    serror("fail to allocate jByteArray for Instream.Read");
    return -1;
  }

  if (off < 0 || maxLen <= 0 || length == maxLen)
    exception = callMethod(m_env, &ret, m_obj, TINSTREAM_CLS, TINSTREAM_READ_METHD,
                  "([B)I", false, jBuf);
  else
    exception = callMethod(m_env, &ret, m_obj, TINSTREAM_CLS, TINSTREAM_READ_METHD,
                  "([BII)I", false, jBuf, off, maxLen);
  if (exception != NULL) {
    m_env->DeleteLocalRef(jBuf);
    serror("fail to call InStream.Read()");
    printException(m_env, exception);
    return -1;
  }
  rdSz = ret.i;
  if (rdSz > 0) {
    m_env->GetByteArrayRegion(jBuf, 0, length, (jbyte*) buff);
  }
  m_env->DeleteLocalRef(jBuf);
  return rdSz;
}

void InStream::close()
{
  callMethod(m_env, NULL, m_obj, TINSTREAM_CLS, TINSTREAM_CLOSE_METHD, 
      "()V", false);
}

void InStream::seek(long pos)
{
  callMethod(m_env, NULL, m_obj, TINSTREAM_CLS, TINSTREAM_SEEK_METHD, 
      "(J)V", false, (jlong) pos);
}

long InStream::skip(long n)
{
  jthrowable exception;
  jvalue ret;
  
  exception = callMethod(m_env, NULL, m_obj, TINSTREAM_CLS, 
                TINSTREAM_SKIP_METHD, "(J)J", false, (jlong) n);
  if (exception != NULL) {
    serror("fail to call InStream.skip()");
    printException(m_env, exception);
    return -1;
  }
  return ret.j;
}

jthrowable enumObjReadType(JNIEnv *env, jobject *objOut, ReadType readType)
{
  const char *valueName;
  switch (readType) {
    case NO_CACHE: 
          valueName = "NO_CACHE";
          break;
    case CACHE:
          valueName = "CACHE";
          break;
    case CACHE_PROMOTE:
          valueName = "CACHE_PROMOTE";
          break;
    default:
          return newRuntimeException(env, "invalid readType");
  }
  return getEnumObject(env, objOut, TREADT_CLS, valueName);
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
