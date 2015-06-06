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

#include <string>
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
    verror("fail to call TachyonFS.get() for %s\n", masterUri);
    printException(env, exception);
    return NULL;
  }
  if (ret.l == NULL)
    return NULL;
  return new TachyonClient(env, ret.l);
}

jTachyonFile TachyonClient::getFile(const char * path)
{
  jthrowable exception;
  jvalue ret;
  jstring jPathStr;
  
  jPathStr  = m_env->NewStringUTF(path);
  if (jPathStr == NULL) {
    serror("fail to allocate path string");
    return NULL;
  }

  exception = callMethod(m_env, &ret, m_obj, TFS_CLS, TFS_GET_FILE_METHD, 
                "(Ljava/lang/String;)Ltachyon/client/TachyonFile;", false, jPathStr);
  m_env->DeleteLocalRef(jPathStr); 
  if (exception != NULL) {
    verror("fail to call TachyonFS.getFile() for %s\n", path);
    printException(m_env, exception);
    return NULL;
  }
  if (ret.l == NULL)
    return NULL;
  return new TachyonFile(m_env, ret.l);
}

jTachyonFile TachyonClient::getFile(int fid)
{
  jthrowable exception;
  jvalue ret;
  
  exception = callMethod(m_env, &ret, m_obj, TFS_CLS, TFS_GET_FILE_METHD, 
                "(I)Ltachyon/client/TachyonFile;", false, (jint) fid);
  if (exception != NULL) {
    verror("fail to call TachyonFS.getFile() for fid %d\n", fid);
    printException(m_env, exception);
    return NULL;
  }
  if (ret.l == NULL)
    return NULL;
  return new TachyonFile(m_env, ret.l);
}

jTachyonFile TachyonClient::getFile(int fid, bool useCachedMetadata)
{
  jthrowable exception;
  jvalue ret;
  
  exception = callMethod(m_env, &ret, m_obj, TFS_CLS, TFS_GET_FILE_METHD, 
                "(IZ)Ltachyon/client/TachyonFile;", false, 
                (jint) fid, (jboolean) useCachedMetadata);
  if (exception != NULL) {
    verror("fail to call TachyonFS.getFile() for fid %d\n", fid);
    printException(m_env, exception);
    return NULL;
  }
  if (ret.l == NULL)
    return NULL;
  return new TachyonFile(m_env, ret.l);
}

int TachyonClient::getFileId(const char *path)
{
  jthrowable exception;
  jvalue ret;
  jTachyonURI uri = TachyonURI::newURI(path);
  if (uri == NULL)
    return -1;
  exception = callMethod(m_env, &ret, m_obj, TFS_CLS, TFS_GET_FILEID_METHD, 
                "(Ltachyon/TachyonURI;)I", false, uri->getJObj());
  delete uri;
  if (exception != NULL) {
    verror("fail to call TachyonFS.getFileId() for %s\n", path);
    printException(m_env, exception);
    return NULL;
  }
  return ret.i;
}

int TachyonClient::createFile(const char * path)
{
  jthrowable exception;
  jvalue ret;
  jstring jPathStr;
  
  jPathStr  = m_env->NewStringUTF(path);
  if (jPathStr == NULL) {
    serror("fail to allocate path string");
    return 0;
  }

  exception = callMethod(m_env, &ret, m_obj, TFS_CLS, TFS_CREATE_FILE_METHD, 
                "(Ljava/lang/String;)I", false, jPathStr);
  m_env->DeleteLocalRef(jPathStr); 
  if (exception != NULL) {
    verror("fail to call TachyonFS.createFile() for %s\n", path);
    printException(m_env, exception);
    return 0;
  }
  return ret.i;
}

bool TachyonClient::mkdir(const char *path)
{
  jthrowable exception;
  jvalue ret;
  jTachyonURI uri = TachyonURI::newURI(path);
  if (uri == NULL)
    return false;
  
  exception = callMethod(m_env, &ret, m_obj, TFS_CLS, TFS_MKDIR_METHD, 
                "(Ltachyon/TachyonURI;)Z", false, uri->getJObj());
  delete uri;
  if (exception != NULL) {
    verror("fail to call TachyonFS.mkdir() for %s\n", path);
    printException(m_env, exception);
    return false;
  }
  return ret.z;
}

bool TachyonClient::mkdirs(const char *path, bool recursive)
{
  jthrowable exception;
  jvalue ret;
  jTachyonURI uri = TachyonURI::newURI(path);
  if (uri == NULL)
    return false;
  
  exception = callMethod(m_env, &ret, m_obj, TFS_CLS, TFS_MKDIRS_METHD, 
                "(Ltachyon/TachyonURI;Z)Z", false, uri->getJObj(), (jboolean) recursive);
  delete uri;
  if (exception != NULL) {
    verror("fail to call TachyonFS.mkdir() for %s", path);
    printException(m_env, exception);
    return false;
  }
  return ret.z;
}

bool TachyonClient::deletePath(const char *path, bool recursive)
{
  jthrowable exception;
  jvalue ret;
  jstring jPathStr;
  
  jPathStr  = m_env->NewStringUTF(path);
  if (jPathStr == NULL) {
    serror("fail to allocate path string");
    return false;
  }

  exception = callMethod(m_env, &ret, m_obj, TFS_CLS, TFS_DELETE_FILE_METHD, 
                "(Ljava/lang/String;Z)Z", false, jPathStr, (jboolean) recursive);
  m_env->DeleteLocalRef(jPathStr); 
  if (exception != NULL) {
    verror("fail to call TachyonFS.delete() for %s\n", path);
    printException(m_env, exception);
    return false;
  }
  return ret.z;
}

bool TachyonClient::deletePath(int fid, bool recursive)
{
  jthrowable exception;
  jvalue ret;
  
  exception = callMethod(m_env, &ret, m_obj, TFS_CLS, TFS_DELETE_FILE_METHD, 
                "(IZ)Z", false, (jint) fid, (jboolean) recursive);
  if (exception != NULL) {
    verror("fail to call TachyonFS.delete() for fid %d\n", fid);
    printException(m_env, exception);
    return NULL;
  }
  return ret.z;
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

bool TachyonFile::isFile()
{
  jthrowable exception;
  jvalue ret;
  
  exception = callMethod(m_env, &ret, m_obj, TFILE_CLS, TFILE_ISFILE_METHD,
                "()Z", false);
  if (exception != NULL) {
    serror("fail to call TachyonFile.isFile()");
    printException(m_env, exception);
    return false; 
  }
  return ret.z;
}

char * TachyonFile::getPath()
{
  jthrowable exception;
  jvalue ret;
  jstring jpath;
  const char *path;
  char *retPath;

  exception = callMethod(m_env, &ret, m_obj, TFILE_CLS, TFILE_PATH_METHD, 
                "()Ljava/lang/String;", false);
  if (exception != NULL) {
    serror("fail to call TachyonFile.getPath()");
    printException(m_env, exception);
    return NULL;
  }
  if (ret.l == NULL)
    return NULL;
  jpath = (jstring) ret.l;
  path = m_env->GetStringUTFChars(jpath, 0);
  retPath = strdup(path);
  m_env->ReleaseStringUTFChars(jpath, path);
  return retPath;
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
  if (ret.l == NULL)
    return NULL;
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
  if (ret.l == NULL)
    return NULL;
  return new InStream(m_env, ret.l);
}

jOutStream TachyonFile::getOutStream(WriteType writeType)
{
  jthrowable exception;
  jvalue ret;
  jobject eobj;

  exception = enumObjWriteType(m_env, &eobj, writeType);
  if (exception != NULL) {
    serror("fail to get enum obj for write type");
    printException(m_env, exception);
    return NULL;
  }
  
  exception = callMethod(m_env, &ret, m_obj, TFILE_CLS, TFILE_GOS_METHD, 
                "(Ltachyon/client/WriteType;)Ltachyon/client/OutStream;", false, eobj);
  if (exception != NULL) {
    serror("fail to call TachyonFile.getOutStream()");
    printException(m_env, exception);
    return NULL;
  }
  if (ret.l == NULL)
    return NULL;
  return new OutStream(m_env, ret.l);
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
  jthrowable exception;
  callMethod(m_env, NULL, m_obj, TBBUF_CLS, TBBUF_CLOSE_METHD, "()V", false);
  exception = getAndClearException(m_env);
  if (exception != NULL) {
    printException(m_env, exception);
  }
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
  if (ret.l == NULL)
    return NULL;
  return new ByteBuffer(env, ret.l);
}

int InStream::read() 
{
  jthrowable exception;
  jvalue ret;

  exception = callMethod(m_env, &ret, m_obj, TISTREAM_CLS, TISTREAM_READ_METHD,
                "()I", false);
  if (exception != NULL) {
    serror("fail to call InStream.Read()");
    printException(m_env, exception);
    return 0;
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
    exception = callMethod(m_env, &ret, m_obj, TISTREAM_CLS, TISTREAM_READ_METHD,
                  "([B)I", false, jBuf);
  else
    exception = callMethod(m_env, &ret, m_obj, TISTREAM_CLS, TISTREAM_READ_METHD,
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
  callMethod(m_env, NULL, m_obj, TISTREAM_CLS, TISTREAM_CLOSE_METHD, 
      "()V", false);
}

void InStream::seek(long pos)
{
  callMethod(m_env, NULL, m_obj, TISTREAM_CLS, TISTREAM_SEEK_METHD, 
      "(J)V", false, (jlong) pos);
}

long InStream::skip(long n)
{
  jthrowable exception;
  jvalue ret;
  
  exception = callMethod(m_env, NULL, m_obj, TISTREAM_CLS, 
                TISTREAM_SKIP_METHD, "(J)J", false, (jlong) n);
  if (exception != NULL) {
    serror("fail to call InStream.skip()");
    printException(m_env, exception);
    return -1;
  }
  return ret.j;
}

void OutStream::cancel()
{
  callMethod(m_env, NULL, m_obj, TOSTREAM_CLS, TOSTREAM_CANCEL_METHD, 
      "()V", false);
}

void OutStream::close()
{
  callMethod(m_env, NULL, m_obj, TOSTREAM_CLS, TOSTREAM_CLOSE_METHD, 
      "()V", false);
}

void OutStream::flush()
{
  callMethod(m_env, NULL, m_obj, TOSTREAM_CLS, TOSTREAM_FLUSH_METHD, 
      "()V", false);
}


void OutStream::write(int byte) 
{
  callMethod(m_env, NULL, m_obj, TOSTREAM_CLS, TOSTREAM_WRITE_METHD,
      "(I)V", false, (jint) byte);
}

void OutStream::write(const void *buff, int length)
{
  write(buff, length, 0, length);
}

void OutStream::write(const void *buff, int length, int off, int maxLen)
{
  jthrowable exception;
  jbyteArray jBuf;

  jBuf = m_env->NewByteArray(length);
  if (jBuf == NULL) {
    serror("fail to allocate jByteArray for OutStream.Write");
    return;
  }

  m_env->SetByteArrayRegion(jBuf, 0, length, (jbyte*) buff);

  char *jbuff = (char *) malloc(length * sizeof(char));
  m_env->GetByteArrayRegion(jBuf, 0, length, (jbyte*) jbuff);
  // printf("byte array in write: %s\n", jbuff);

  if (off < 0 || maxLen <= 0 || length == maxLen)
    exception = callMethod(m_env, NULL, m_obj, TOSTREAM_CLS, TOSTREAM_WRITE_METHD,
                  "([B)V", false, jBuf);
  else
    exception = callMethod(m_env, NULL, m_obj, TOSTREAM_CLS, TOSTREAM_WRITE_METHD,
                  "([BII)V", false, jBuf, (jint) off, (jint) maxLen);
  m_env->DeleteLocalRef(jBuf);
  if (exception != NULL) {
    serror("fail to call OutStream.Write()");
    printException(m_env, exception);
  }
}

jTachyonURI TachyonURI::newURI(const char *pathStr)
{
  JNIEnv *env = getJNIEnv();
  if (env == NULL) {
    return NULL;
  }

  jthrowable exception;
  jobject retObj;
  
  jstring jPathStr = env->NewStringUTF(pathStr);
  if (jPathStr == NULL) {
    serror("fail to allocate path string");
    return NULL;
  }

  exception = newClassObject(env, &retObj, TURI_CLS,
                  "(Ljava/lang/String;)V", jPathStr);
  env->DeleteLocalRef(jPathStr);
  if (exception != NULL) {
    serror("fail to new TachyonURI");
    printException(env, exception);
    return NULL;
  }
  return new TachyonURI(env, retObj);
}

jTachyonURI TachyonURI::newURI(const char *scheme, const char *authority, const char *path)
{
  JNIEnv *env = getJNIEnv();
  if (env == NULL) {
    return NULL;
  }

  jthrowable exception;
  jobject retObj;
  jstring jscheme, jauthority, jpath;

  jscheme = env->NewStringUTF(scheme);
  if (jscheme == NULL) {
    serror("fail to allocate scheme string");
    return NULL;
  }
  jauthority = env->NewStringUTF(authority);
  if (jauthority == NULL) {
    env->DeleteLocalRef(jscheme);
    serror("fail to allocate authority string");
    return NULL;
  }
  jpath = env->NewStringUTF(path);
  if (jpath == NULL) {
    env->DeleteLocalRef(jscheme);
    env->DeleteLocalRef(jauthority);
    serror("fail to allocate path string");
    return NULL;
  }

  exception = newClassObject(env, &retObj, TURI_CLS,
                  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", 
                  jscheme, jauthority, jpath);
  env->DeleteLocalRef(jscheme);
  env->DeleteLocalRef(jauthority);
  env->DeleteLocalRef(jpath);
  if (exception != NULL) {
    serror("fail to new TachyonURI");
    printException(env, exception);
    return NULL;
  }
  return new TachyonURI(env, retObj);
}

jTachyonKV TachyonKV::createKV(jTachyonClient client)
{
  return createKV(client, NULL);
}

jTachyonKV TachyonKV::createKV(jTachyonClient client, const char *kvStore)
{
  JNIEnv *env = client->getJEnv();
  jobject retObj;
  jthrowable exception;

  if (kvStore == NULL || strlen(kvStore) == 0) {
    exception = newClassObject(env, &retObj, TKV_CLS,
                  "(Ltachyon/client/TachyonFS;)V", 
                  client->getJObj());
  } else {
    jstring jKVStr = env->NewStringUTF(kvStore);
    if (jKVStr == NULL) {
      serror("fail to allocate kvstore string");
      return NULL;
    }
    exception = newClassObject(env, &retObj, TKV_CLS,
                  "(Ltachyon/client/TachyonFS;Ljava/lang/String;)V", 
                  client->getJObj(), jKVStr);
    env->DeleteLocalRef(jKVStr);
  }
  if (exception != NULL) {
    serror("fail to createKV");
    printException(env, exception);
    return NULL;
  }
  if (retObj == NULL)
    return NULL;
  return new TachyonKV(env, retObj);
}

bool TachyonKV::init()
{
  jthrowable exception;
  jvalue ret;
  callMethod(m_env, &ret, m_obj, TKV_CLS, TKV_INIT_METHD, "()Z", false);
  exception = getAndClearException(m_env);
  if (exception != NULL) {
    printException(m_env, exception);
  }
  return ret.i;
}

int TachyonKV::get(const char *key, uint32_t keylen, char *buff, uint32_t valuelen)
{
  jthrowable exception;
  jbyteArray jBuf;
  jvalue ret;
  int rdSz;

  std::string skey(key, keylen);
  jstring jKeyStr = m_env->NewStringUTF(skey.c_str());
  if (jKeyStr == NULL) {
    serror("fail to allocate key string");
    return NULL;
  }

  jBuf = m_env->NewByteArray(valuelen);
  if (jBuf == NULL) {
    serror("fail to allocate jByteArray for TachyonKV.get");
    return -1;
  }
  exception = callMethod(m_env, &ret, m_obj, TKV_CLS, TKV_GET_METHD,
          "(Ljava/lang/String;[B)I", false, jKeyStr, jBuf);
  m_env->DeleteLocalRef(jKeyStr);
  if (exception != NULL) {
    m_env->DeleteLocalRef(jBuf);
    serror("fail to call TachyonKV.get()");
    printException(m_env, exception);
    return -1;
  }
  rdSz = ret.i;
  if (rdSz > 0) {
    m_env->GetByteArrayRegion(jBuf, 0, valuelen, (jbyte*) buff);
  }
  m_env->DeleteLocalRef(jBuf);
  return rdSz;
}

void TachyonKV::set(const char *key, uint32_t keylen, const char *buff, uint32_t valuelen)
{
  jthrowable exception;
  jbyteArray jBuf;

  std::string skey(key, keylen);
  jstring jKeyStr = m_env->NewStringUTF(skey.c_str());
  if (jKeyStr == NULL) {
    serror("fail to allocate key string");
    return;
  }

  jBuf = m_env->NewByteArray(valuelen);
  if (jBuf == NULL) {
    serror("fail to allocate jByteArray for TachyonKV.set");
    return;
  }
  m_env->SetByteArrayRegion(jBuf, 0, valuelen, (jbyte*) buff);

  exception = callMethod(m_env, NULL, m_obj, TKV_CLS, TKV_SET_METHD,
          "(Ljava/lang/String;[B)V", false, jKeyStr, jBuf);
  m_env->DeleteLocalRef(jKeyStr);
  if (exception != NULL) {
    m_env->DeleteLocalRef(jBuf);
    serror("fail to call TachyonKV.set()");
    printException(m_env, exception);
  }
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

jthrowable enumObjWriteType(JNIEnv *env, jobject *objOut, WriteType writeType)
{
  const char *valueName;
  switch (writeType) {
    case ASYNC_THROUGH: 
          valueName = "ASYNC_THROUGH";
          break;
    case CACHE_THROUGH:
          valueName = "CACHE_THROUGH";
          break;
    case MUST_CACHE:
          valueName = "MUST_CACHE";
          break;
    case THROUGH:
          valueName = "THROUGH";
          break;
    case TRY_CACHE:
          valueName = "TRY_CACHE";
          break;
    default:
          return newRuntimeException(env, "invalid writeType");
  }
  return getEnumObject(env, objOut, TWRITET_CLS, valueName);
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
