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

jTachyonClient TachyonClient::copyClient(jTachyonClient client)
{
  return new TachyonClient(client->getJEnv(), client->getJObj());
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
    return 0;
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

bool TachyonFile::isComplete()
{
  jthrowable exception;
  jvalue ret;

  exception = callMethod(m_env, &ret, m_obj, TFILE_CLS, TFILE_ISCOMPLETE_METHD,
                "()Z", false);
  if (exception != NULL) {
    serror("fail to call TachyonFile.isComplete()");
    printException(m_env, exception);
    return false;
  }
  return ret.z;
}

bool TachyonFile::isDirectory()
{
  jthrowable exception;
  jvalue ret;

  exception = callMethod(m_env, &ret, m_obj, TFILE_CLS, TFILE_ISDIRECTORY_METHD,
                "()Z", false);
  if (exception != NULL) {
    serror("fail to call TachyonFile.isDirectory()");
    printException(m_env, exception);
    return false;
  }
  return ret.z;
}

bool TachyonFile::isInMemory()
{
  jthrowable exception;
  jvalue ret;

  exception = callMethod(m_env, &ret, m_obj, TFILE_CLS, TFILE_ISINMEMORY_METHD,
                "()Z", false);
  if (exception != NULL) {
    serror("fail to call TachyonFile.isInMemory()");
    printException(m_env, exception);
    return false;
  }
  return ret.z;
}

bool TachyonFile::needPin()
{
  jthrowable exception;
  jvalue ret;

  exception = callMethod(m_env, &ret, m_obj, TFILE_CLS, TFILE_NEEDPIN_METHD,
                "()Z", false);
  if (exception != NULL) {
    serror("fail to call TachyonFile.needPin()");
    printException(m_env, exception);
    return false;
  }
  return ret.z;
}

bool TachyonFile::recache()
{
  jthrowable exception;
  jvalue ret;

  exception = callMethod(m_env, &ret, m_obj, TFILE_CLS, TFILE_RECACHE_METHD,
                "()Z", false);
  if (exception != NULL) {
    serror("fail to call TachyonFile.recache()");
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

//////////////////////////////////////////
//InStream
//////////////////////////////////////////

int InStream::readImpl(const char* clsname) 
{
  jthrowable exception;
  jvalue ret;

  exception = callMethod(m_env, &ret, m_obj, clsname, TISTREAM_READ_METHD,
                "()I", false);
  if (exception != NULL) {
    serror("fail to call InStream.Read()");
    printException(m_env, exception);
    return 0;
  }
  return ret.i;
}

int InStream::readImpl(const char* clsname, void *buff, int length, int off, int maxLen)
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
    exception = callMethod(m_env, &ret, m_obj, clsname, TISTREAM_READ_METHD,
                  "([B)I", false, jBuf);
  else
    exception = callMethod(m_env, &ret, m_obj, clsname, TISTREAM_READ_METHD,
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

void InStream::closeImpl(const char* clsname)
{
  callMethod(m_env, NULL, m_obj, clsname, TISTREAM_CLOSE_METHD, 
      "()V", false);
}

void InStream::seekImpl(const char* clsname, long pos)
{
  callMethod(m_env, NULL, m_obj, clsname, TISTREAM_SEEK_METHD, 
      "(J)V", false, (jlong) pos);
}

long InStream::skipImpl(const char* clsname, long n)
{
  jthrowable exception;
  jvalue ret;
  
  exception = callMethod(m_env, NULL, m_obj, clsname, 
                TISTREAM_SKIP_METHD, "(J)J", false, (jlong) n);
  if (exception != NULL) {
    serror("fail to call InStream.skip()");
    printException(m_env, exception);
    return -1;
  }
  return ret.j;
}


// Call the templates

int InStream::read()
{
  return readImpl(TISTREAM_CLS);
}

int InStream::read(void *buff, int length, int off, int maxLen) 
{
  return readImpl(TISTREAM_CLS, buff, length, off, maxLen);
}

int InStream::read(void *buff, int length)
{
  return read(buff, length, 0, length);
}

void InStream::close()
{
  closeImpl(TISTREAM_CLS);
}

void InStream::seek(long pos)
{
  seekImpl(TISTREAM_CLS, pos);
}

long InStream::skip(long n)
{
  return skipImpl(TISTREAM_CLS, n);
}

//////////////////////////////////////////
// FileInStream
//////////////////////////////////////////

int FileInStream::read()
{
  return readImpl(TFILE_ISTREAM_CLS);
}

int FileInStream::read(void *buff, int length, int off, int maxLen) 
{
  return readImpl(TFILE_ISTREAM_CLS, buff, length, off, maxLen);
}

void FileInStream::close()
{
  closeImpl(TFILE_ISTREAM_CLS);
}

long FileInStream::skip(long n)
{
  return skipImpl(TFILE_ISTREAM_CLS, n);
}

void FileInStream::seek(long pos)
{
  seekImpl(TFILE_ISTREAM_CLS, pos);
}

//////////////////////////////////////////
// EmptyBlockInStream
//////////////////////////////////////////

int EmptyBlockInStream::read()
{
  return readImpl(TEMPTY_BLOCK_ISTREAM_CLS);
}

int EmptyBlockInStream::read(void *buff, int length, int off, int maxLen) 
{
  return readImpl(TEMPTY_BLOCK_ISTREAM_CLS, buff, length, off, maxLen);
}

void EmptyBlockInStream::close()
{
  closeImpl(TEMPTY_BLOCK_ISTREAM_CLS);
}

long EmptyBlockInStream::skip(long n)
{
  return skipImpl(TEMPTY_BLOCK_ISTREAM_CLS, n);
}

void EmptyBlockInStream::seek(long pos)
{
  seekImpl(TEMPTY_BLOCK_ISTREAM_CLS, pos);
}

//////////////////////////////////////////
// LocalBlockInStream
//////////////////////////////////////////

int LocalBlockInStream::read()
{
  return readImpl(TLOCAL_BLOCK_ISTREAM_CLS);
}

int LocalBlockInStream::read(void *buff, int length, int off, int maxLen) 
{
  return readImpl(TLOCAL_BLOCK_ISTREAM_CLS, buff, length, off, maxLen);
}

void LocalBlockInStream::close()
{
  closeImpl(TLOCAL_BLOCK_ISTREAM_CLS);
}

long LocalBlockInStream::skip(long n)
{
  return skipImpl(TLOCAL_BLOCK_ISTREAM_CLS, n);
}

void LocalBlockInStream::seek(long pos)
{
  seekImpl(TLOCAL_BLOCK_ISTREAM_CLS, pos);
}

//////////////////////////////////////////
// RemoteBlockInStream
//////////////////////////////////////////

int RemoteBlockInStream::read()
{
  return readImpl(TREMOTE_BLOCK_ISTREAM_CLS);
}

int RemoteBlockInStream::read(void *buff, int length, int off, int maxLen) 
{
  return readImpl(TREMOTE_BLOCK_ISTREAM_CLS, buff, length, off, maxLen);
}

void RemoteBlockInStream::close()
{
  closeImpl(TREMOTE_BLOCK_ISTREAM_CLS);
}

long RemoteBlockInStream::skip(long n)
{
  return skipImpl(TREMOTE_BLOCK_ISTREAM_CLS, n);
}

void RemoteBlockInStream::seek(long pos)
{
  seekImpl(TREMOTE_BLOCK_ISTREAM_CLS, pos);
}

//////////////////////////////////////////
// OutStream
//////////////////////////////////////////

void OutStream::cancelImpl(const char* clsname)
{
  callMethod(m_env, NULL, m_obj, clsname, TOSTREAM_CANCEL_METHD, 
      "()V", false);
}

void OutStream::closeImpl(const char* clsname)
{
  callMethod(m_env, NULL, m_obj, clsname, TOSTREAM_CLOSE_METHD, 
      "()V", false);
}

void OutStream::flushImpl(const char* clsname)
{
  callMethod(m_env, NULL, m_obj, clsname, TOSTREAM_FLUSH_METHD, 
      "()V", false);
}


void OutStream::writeImpl(const char* clsname, int byte) 
{
  callMethod(m_env, NULL, m_obj, clsname, TOSTREAM_WRITE_METHD,
      "(I)V", false, (jint) byte);
}

void OutStream::writeImpl(const char* clsname, const void *buff, int length, int off, int maxLen)
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
    exception = callMethod(m_env, NULL, m_obj, clsname, TOSTREAM_WRITE_METHD,
                  "([B)V", false, jBuf);
  else
    exception = callMethod(m_env, NULL, m_obj, clsname, TOSTREAM_WRITE_METHD,
                  "([BII)V", false, jBuf, (jint) off, (jint) maxLen);
  m_env->DeleteLocalRef(jBuf);
  if (exception != NULL) {
    serror("fail to call OutStream.Write()");
    printException(m_env, exception);
  }
}

// Call the templates

void OutStream::cancel() 
{
  cancelImpl(TOSTREAM_CLS);
}

void OutStream::close()
{
  closeImpl(TOSTREAM_CLS);
}

void OutStream::flush()
{
  flushImpl(TOSTREAM_CLS);
}

void OutStream::write(const void *buff, int length, int off, int maxLen)
{
  writeImpl(TOSTREAM_CLS, buff, length, off, maxLen);
}

void OutStream::write(const void *buff, int length)
{
  write(buff, length, 0, length);
}

//////////////////////////////////////////
// FileOutStream
//////////////////////////////////////////

void FileOutStream::cancel() 
{
  cancelImpl(TFILE_OSTREAM_CLS);
}

void FileOutStream::close()
{
  closeImpl(TFILE_OSTREAM_CLS);
}

void FileOutStream::flush()
{
  flushImpl(TFILE_OSTREAM_CLS);
}

void FileOutStream::write(const void *buff, int length, int off, int maxLen)
{
  writeImpl(TFILE_OSTREAM_CLS, buff, length, off, maxLen);
}

//////////////////////////////////////////
// BlockOutStream
//////////////////////////////////////////

void BlockOutStream::cancel() 
{
  cancelImpl(TBLOCK_OSTREAM_CLS);
}

void BlockOutStream::close()
{
  closeImpl(TBLOCK_OSTREAM_CLS);
}

void BlockOutStream::flush()
{
  flushImpl(TBLOCK_OSTREAM_CLS);
}

void BlockOutStream::write(const void *buff, int length, int off, int maxLen)
{
  writeImpl(TBLOCK_OSTREAM_CLS, buff, length, off, maxLen);
}

//////////////////////////////////////////
// TachyonURI
//////////////////////////////////////////

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

jTachyonKV TachyonKV::createKV(jTachyonClient client, ReadType readType, 
        WriteType writeType, long blockSizeByte, const char *kvStore)
{
  JNIEnv *env = client->getJEnv();
  jobject retObj, ereadtObj, ewritetObj;
  jthrowable exception;

  exception = enumObjReadType(env, &ereadtObj, readType);
  if (exception != NULL) {
    serror("fail to get enum obj for read type");
    printException(env, exception);
    return NULL;
  }
  exception = enumObjWriteType(env, &ewritetObj, writeType);
  if (exception != NULL) {
    serror("fail to get enum obj for write type");
    printException(env, exception);
    return NULL;
  }

  if (kvStore == NULL || strlen(kvStore) == 0) {
    exception = newClassObject(env, &retObj, TKV_CLS,
                  "(Ltachyon/client/TachyonFS;Ltachyon/client/ReadType;Ltachyon/client/WriteType;J)V", 
                  client->getJObj(), ereadtObj, ewritetObj, (jlong) blockSizeByte);
  } else {
    jstring jKVStr = env->NewStringUTF(kvStore);
    if (jKVStr == NULL) {
      serror("fail to allocate kvstore string");
      return NULL;
    }
    exception = newClassObject(env, &retObj, TKV_CLS,
                  "(Ltachyon/client/TachyonFS;Ltachyon/client/ReadType;Ltachyon/client/WriteType;JLjava/lang/String;)V", 
                  client->getJObj(), ereadtObj, ewritetObj, (jlong) blockSizeByte, jKVStr);
    env->DeleteLocalRef(jKVStr);
  }
  if (exception != NULL) {
    serror("fail to createKV");
    printException(env, exception);
    return NULL;
  }
  if (retObj == NULL)
    return NULL;
  return new TachyonKV(client, retObj);
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
  int rdSz;
  jobject jDbuff;
  jthrowable exception;
  jvalue ret;

  exception = callMethod(m_env, &ret, m_obj, TKV_CLS, TKV_GRBUFF_METHD,
          "()Ljava/nio/ByteBuffer;", false);
  if (exception != NULL) {
    serror("fail to get read bytebuffer");
    printException(m_env, exception);
  }
  jDbuff = ret.l;

  std::string skey(key, keylen);
  jstring jKeyStr = m_env->NewStringUTF(skey.c_str());
  if (jKeyStr == NULL) {
    serror("fail to allocate key string");
    return -1;
  }
  exception = callMethod(m_env, &ret, m_obj, TKV_CLS, TKV_RBUFF_METHD,
          "(Ljava/lang/String;Ljava/nio/ByteBuffer;)I", false, jKeyStr, jDbuff);
  m_env->DeleteLocalRef(jKeyStr);
  if (exception != NULL) {
    serror("fail to call TachyonKV.readBuffer()");
    printException(m_env, exception);
    return -1;
  }
  char *cDbuff = (char *) m_env->GetDirectBufferAddress(jDbuff);
  if (cDbuff == NULL) {
    serror("fail to get direct buffer address");
    return -1;
  }
  rdSz = ret.i;
  strncpy(buff, cDbuff, valuelen);

  /*
  jthrowable exception;
  jbyteArray jBuf;
  jvalue ret;

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
    // serror("fail to call TachyonKV.get()");
    printException(m_env, exception);
    return -1;
  }
  rdSz = ret.i;
  if (rdSz > 0) {
    m_env->GetByteArrayRegion(jBuf, 0, valuelen, (jbyte*) buff);
  }
  m_env->DeleteLocalRef(jBuf);
  */
  return rdSz;
}

void TachyonKV::set(const char *key, uint32_t keylen, const char *buff, uint32_t valuelen)
{
  jobject jDbuff;
  jthrowable exception;

  jDbuff = m_env->NewDirectByteBuffer((void *) buff, valuelen);
  if (jDbuff == NULL) {
    serror("fail to create direct byte buffer");
    return;
  }

  std::string skey(key, keylen);
  jstring jKeyStr = m_env->NewStringUTF(skey.c_str());
  if (jKeyStr == NULL) {
    serror("fail to allocate key string");
    return;
  }
  exception = callMethod(m_env, NULL, m_obj, TKV_CLS, TKV_WBUFF_METHD,
          "(Ljava/lang/String;Ljava/nio/ByteBuffer;)V", false, jKeyStr, jDbuff);
  m_env->DeleteLocalRef(jKeyStr);
  if (exception != NULL) {
    serror("fail to call TachyonKV.writeBuffer()");
    printException(m_env, exception);
  }

  /*
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
    // serror("fail to call TachyonKV.set()");
    printException(m_env, exception);
  }
  */
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
