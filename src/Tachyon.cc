/**
 *  @author        Ryan Huang <ryanhuang@cs.ucsd.edu>
 *  @organization  University of California, San Diego
 * 
 * Tachyon C/C++ APIs
 *
 */

#include "Tachyon.h"
#include "Util.h"

#include <string>
#include <string.h>
#include <stdlib.h>

using namespace tachyon;
using namespace tachyon::jni;

jTachyonClient TachyonClient::createClient(const char *masterUri)
{
  Env env;
  jstring jPathStr; 
  jvalue ret;

  jPathStr = env.newStringUTF(masterUri, "masterUri");
  // might throw exception, caller needs to handle it
  env.callStaticMethod(&ret, TFS_CLS, "get", 
                "(Ljava/lang/String;)Ltachyon/client/TachyonFS;", jPathStr);
  
  env->DeleteLocalRef(jPathStr); 
  return new TachyonClient(env, ret.l);
}

jTachyonClient TachyonClient::copyClient(jTachyonClient client)
{
  return new TachyonClient(client->getEnv(), client->getJObj());
}

jTachyonFile TachyonClient::getFile(const char * path)
{
  jvalue ret;
  jstring jPathStr;
  
  jPathStr = m_env.newStringUTF(path, "path");
  m_env.callMethod(&ret, m_obj, "getFile", 
                "(Ljava/lang/String;)Ltachyon/client/TachyonFile;", jPathStr);
  m_env->DeleteLocalRef(jPathStr); 
  if (ret.l == NULL)
    return NULL;
  return new TachyonFile(m_env, ret.l);
}

jTachyonFile TachyonClient::getFile(int fid)
{
  jvalue ret;
  m_env.callMethod(&ret, m_obj, "getFile", 
                "(I)Ltachyon/client/TachyonFile;", (jint) fid);
  if (ret.l == NULL)
    return NULL;
  return new TachyonFile(m_env, ret.l);
}

jTachyonFile TachyonClient::getFile(int fid, bool useCachedMetadata)
{
  jvalue ret;
  m_env.callMethod(&ret, m_obj, "getFile", 
                "(IZ)Ltachyon/client/TachyonFile;",
                (jint) fid, (jboolean) useCachedMetadata);
  if (ret.l == NULL)
    return NULL;
  return new TachyonFile(m_env, ret.l);
}

int TachyonClient::getFileId(const char *path)
{
  jvalue ret;
  jTachyonURI uri = TachyonURI::newURI(path);
  if (uri == NULL)
    return -1;
  m_env.callMethod(&ret, m_obj, "getFileId", 
                    "(Ltachyon/TachyonURI;)I", uri->getJObj());
  delete uri;
  return ret.i;
}

int TachyonClient::createFile(const char * path)
{
  jvalue ret;
  jstring jPathStr;
  
  jPathStr = m_env.newStringUTF(path, "path");

  m_env.callMethod(&ret, m_obj, "createFile", 
                    "(Ljava/lang/String;)I", jPathStr);
  m_env->DeleteLocalRef(jPathStr); 
  return ret.i;
}

bool TachyonClient::mkdir(const char *path)
{
  jvalue ret;
  jTachyonURI uri = TachyonURI::newURI(path);
  if (uri == NULL)
    return false;
  
  m_env.callMethod(&ret, m_obj, "mkdir", 
                "(Ltachyon/TachyonURI;)Z", uri->getJObj());
  delete uri;
  return ret.z;
}

bool TachyonClient::mkdirs(const char *path, bool recursive)
{
  jvalue ret;
  jTachyonURI uri = TachyonURI::newURI(path);
  if (uri == NULL)
    return false;
  
  m_env.callMethod(&ret, m_obj, "mkdirs", 
                "(Ltachyon/TachyonURI;Z)Z", uri->getJObj(), (jboolean) recursive);
  delete uri;
  return ret.z;
}

bool TachyonClient::deletePath(const char *path, bool recursive)
{
  jvalue ret;
  jstring jPathStr;
  
  jPathStr = m_env.newStringUTF(path, "path");
  m_env.callMethod(&ret, m_obj, "delete", 
                    "(Ljava/lang/String;Z)Z", jPathStr, (jboolean) recursive);
  m_env->DeleteLocalRef(jPathStr); 
  return ret.z;
}

bool TachyonClient::deletePath(int fid, bool recursive)
{
  jvalue ret;
  m_env.callMethod(&ret, m_obj, "delete", 
                    "(IZ)Z", (jint) fid, (jboolean) recursive);
  return ret.z;
}

long TachyonFile::length()
{
  jvalue ret;
  m_env.callMethod(&ret, m_obj, "length", "()J");
  return ret.j;
}

bool TachyonFile::isFile()
{
  jvalue ret;
  m_env.callMethod(&ret, m_obj, "isFile", "()Z");
  return ret.z;
}

bool TachyonFile::isComplete()
{
  jvalue ret;
  m_env.callMethod(&ret, m_obj, "isComplete", "()Z");
  return ret.z;
}

bool TachyonFile::isDirectory()
{
  jvalue ret;
  m_env.callMethod(&ret, m_obj, "isDirectory", "()Z");
  return ret.z;
}

bool TachyonFile::isInMemory()
{
  jvalue ret;
  m_env.callMethod(&ret, m_obj, "isInMemory", "()Z");
  return ret.z;
}

bool TachyonFile::needPin()
{
  jvalue ret;
  m_env.callMethod(&ret, m_obj, "needPin", "()Z");
  return ret.z;
}

bool TachyonFile::recache()
{
  jvalue ret;
  m_env.callMethod(&ret, m_obj, "recache", "()Z");
  return ret.z;
}

char * TachyonFile::getPath()
{
  jvalue ret;
  jstring jpath;
  const char *path;
  char *retPath;

  m_env.callMethod(&ret, m_obj, "getPath", "()Ljava/lang/String;");
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
  jvalue ret;
  m_env.callMethod(&ret, m_obj, "readByteBuffer", 
                "(I)Ltachyon/client/TachyonByteBuffer;", (jint) blockIndex);
  if (ret.l == NULL)
    return NULL;
  return new TachyonByteBuffer(m_env, ret.l);
}

jInStream TachyonFile::getInStream(ReadType readType)
{
  jvalue ret;
  jobject eobj;

  eobj = enumObjReadType(m_env, readType);
  m_env.callMethod(&ret, m_obj, "getInStream", 
                "(Ltachyon/client/ReadType;)Ltachyon/client/InStream;", eobj);
  if (ret.l == NULL)
    return NULL;
  return new InStream(m_env, ret.l);
}

jOutStream TachyonFile::getOutStream(WriteType writeType)
{
  jvalue ret;
  jobject eobj;

  eobj = enumObjWriteType(m_env, writeType);
  m_env.callMethod(&ret, m_obj, "getOutStream", 
                "(Ltachyon/client/WriteType;)Ltachyon/client/OutStream;", eobj);
  if (ret.l == NULL)
    return NULL;
  return new OutStream(m_env, ret.l);
}

jByteBuffer TachyonByteBuffer::getData()
{
  jobject ret;
  ret = m_env.getObjectField(m_obj, "mData", "Ljava/nio/ByteBuffer;");
  return new ByteBuffer(m_env, ret);
}

void TachyonByteBuffer::close()
{
  m_env.callMethod(NULL, m_obj, "close", "()V");
}

jByteBuffer ByteBuffer::allocate(int capacity)
{
  Env env;
  jvalue ret;
  env.callStaticMethod(&ret, BBUF_CLS, "allocate", 
                "(I)Ljava/nio/ByteBuffer;", (jint) capacity);
  if (ret.l == NULL)
    return NULL;
  return new ByteBuffer(env, ret.l);
}

//////////////////////////////////////////
//InStream
//////////////////////////////////////////

int InStream::read()
{
  jvalue ret;
  m_env.callMethod(&ret, m_obj, "read", "()I");
  return ret.i;
}

int InStream::read(void *buff, int length, int off, int maxLen) 
{
  jbyteArray jBuf;
  jvalue ret;
  int rdSz;

  try {
    jBuf = m_env.newByteArray(length);
    if (off < 0 || maxLen <= 0 || length == maxLen)
      m_env.callMethod(&ret, m_obj, "read", "([B)I", jBuf);
    else
      m_env.callMethod(&ret, m_obj, "read", "([BII)I", jBuf, off, maxLen);
  } catch (NativeException) {
    if (jBuf != NULL) {
      m_env->DeleteLocalRef(jBuf);
    }
    throw;
  }
  rdSz = ret.i;
  if (rdSz > 0) {
    m_env->GetByteArrayRegion(jBuf, 0, length, (jbyte*) buff);
  }
  m_env->DeleteLocalRef(jBuf);
  return rdSz;
}

int InStream::read(void *buff, int length)
{
  return read(buff, length, 0, length);
}

void InStream::close()
{
  m_env.callMethod(NULL, m_obj, "close", "()V");
}

void InStream::seek(long pos)
{
  m_env.callMethod(NULL, m_obj, "seek", "(J)V", (jlong) pos);
}

long InStream::skip(long n)
{
  jvalue ret;
  m_env.callMethod(NULL, m_obj, "skip", "(J)J", (jlong) n);
  return ret.j;
}

//////////////////////////////////////////
// OutStream
//////////////////////////////////////////

void OutStream::write(int byte) 
{
  m_env.callMethod(NULL, m_obj, "write", "(I)V", (jint) byte);
}

void OutStream::write(const void *buff, int length)
{
  write(buff, length, 0, length);
}

void OutStream::write(const void *buff, int length, 
                          int off, int maxLen)
{
  jthrowable exception;
  jbyteArray jBuf;

  jBuf = m_env.newByteArray(length);
  m_env->SetByteArrayRegion(jBuf, 0, length, (jbyte*) buff);

  char *jbuff = (char *) malloc(length * sizeof(char));
  m_env->GetByteArrayRegion(jBuf, 0, length, (jbyte*) jbuff);
  // printf("byte array in write: %s\n", jbuff);

  if (off < 0 || maxLen <= 0 || length == maxLen)
    m_env.callMethod(NULL, m_obj, "write", "([B)V", jBuf);
  else
    m_env.callMethod(NULL, m_obj, "write", "([BII)V", jBuf, (jint) off, (jint) maxLen);
  m_env->DeleteLocalRef(jBuf);
}

// Call the templates

void OutStream::cancel() 
{
  m_env.callMethod(NULL, m_obj, "cancel", "()V");
}

void OutStream::close()
{
  m_env.callMethod(NULL, m_obj, "close", "()V");
}

void OutStream::flush()
{
  m_env.callMethod(NULL, m_obj, "flush", "()V");
}

//////////////////////////////////////////
// TachyonURI
//////////////////////////////////////////

jTachyonURI TachyonURI::newURI(const char *pathStr)
{
  Env env;
  jobject retObj;
  jstring jPathStr;
  
  jPathStr = env.newStringUTF(pathStr, "path");
  retObj = env.newObject(TURI_CLS, "(Ljava/lang/String;)V", jPathStr);
  env->DeleteLocalRef(jPathStr);
  return new TachyonURI(env, retObj);
}

jTachyonURI TachyonURI::newURI(const char *scheme, const char *authority, const char *path)
{
  Env env;
  jobject retObj;
  jstring jscheme, jauthority, jpath;

  jscheme = env.newStringUTF(scheme, "scheme");
  jauthority = env.newStringUTF(authority, "authority");
  jpath = env.newStringUTF(path, "path");
  retObj = env.newObject(TURI_CLS,
                  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", 
                  jscheme, jauthority, jpath);
  env->DeleteLocalRef(jscheme);
  env->DeleteLocalRef(jauthority);
  env->DeleteLocalRef(jpath);
  return new TachyonURI(env, retObj);
}

jTachyonKV TachyonKV::createKV(jTachyonClient client, ReadType readType, 
        WriteType writeType, long blockSizeByte, const char *kvStore)
{
  Env& env = client->getEnv();
  jobject retObj, ereadtObj, ewritetObj;

  ereadtObj = enumObjReadType(env, readType);
  ewritetObj = enumObjWriteType(env, writeType);

  if (kvStore == NULL || strlen(kvStore) == 0) {
    retObj = env.newObject(TKV_CLS,
                  "(Ltachyon/client/TachyonFS;Ltachyon/client/ReadType;Ltachyon/client/WriteType;J)V", 
                  client->getJObj(), ereadtObj, ewritetObj, (jlong) blockSizeByte);
  } else {
    jstring jKVStr = env.newStringUTF(kvStore, "kvstore");
    retObj = env.newObject(TKV_CLS,
                  "(Ltachyon/client/TachyonFS;Ltachyon/client/ReadType;Ltachyon/client/WriteType;JLjava/lang/String;)V", 
                  client->getJObj(), ereadtObj, ewritetObj, (jlong) blockSizeByte, jKVStr);
    env->DeleteLocalRef(jKVStr);
  }
  if (retObj == NULL)
    return NULL;
  return new TachyonKV(client, retObj);
}

bool TachyonKV::init()
{
  jthrowable exception;
  jvalue ret;
  m_env.callMethod(&ret, m_obj, "init", "()Z");
  return ret.i;
}

int TachyonKV::get(const char *key, uint32_t keylen, char *buff, uint32_t valuelen)
{
  int rdSz;
  jobject jDbuff;
  jvalue ret;

  m_env.callMethod(&ret, m_obj, "getReadBuffer",
          "()Ljava/nio/ByteBuffer;");
  jDbuff = ret.l;

  std::string skey(key, keylen);
  jstring jKeyStr = m_env.newStringUTF(skey.c_str(), "key");
  m_env.callMethod(&ret, m_obj, "readBuffer",
          "(Ljava/lang/String;Ljava/nio/ByteBuffer;)I", jKeyStr, jDbuff);
  m_env->DeleteLocalRef(jKeyStr);
  char *cDbuff = (char *) m_env->GetDirectBufferAddress(jDbuff);
  if (cDbuff == NULL) {
    serror("fail to get direct buffer address");
    return -1;
  }
  rdSz = ret.i;
  strncpy(buff, cDbuff, valuelen);
  return rdSz;
}

void TachyonKV::set(const char *key, uint32_t keylen, const char *buff, uint32_t valuelen)
{
  jobject jDbuff;

  jDbuff = m_env->NewDirectByteBuffer((void *) buff, valuelen);
  if (jDbuff == NULL) {
    serror("fail to create direct byte buffer");
    return;
  }

  std::string skey(key, keylen);
  jstring jKeyStr = m_env.newStringUTF(skey.c_str(), "key");
  m_env.callMethod(NULL, m_obj, "writeBuffer",
          "(Ljava/lang/String;Ljava/nio/ByteBuffer;)V", jKeyStr, jDbuff);
  m_env->DeleteLocalRef(jKeyStr);
}

jobject enumObjReadType(Env& env, ReadType readType)
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
          throw std::runtime_error("invalid readType");
  }
  return env.getEnumObject(TREADT_CLS, valueName);
}

jobject enumObjWriteType(Env& env, WriteType writeType)
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
          throw std::runtime_error("invalid writeType");
  }
  return env.getEnumObject(TWRITET_CLS, valueName);
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
