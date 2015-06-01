/**
 *  @author        Ryan Huang <ryanhuang@cs.ucsd.edu>
 *  @organization  University of California, San Diego
 * 
 * Tachyon C/C++ APIs
 *
 */

#ifndef __TACHYON_H_
#define __TACHYON_H_

#include<jni.h>

#define TFS_CLS                     "tachyon/client/TachyonFS"
#define TFS_GET_METHD               "get"
#define TFS_GET_FILE_METHD          "getFile"
#define TFS_CREATE_FILE_METHD       "createFile"

#define TFILE_CLS                   "tachyon/client/TachyonFile"
#define TFILE_LENGTH_METHD          "length"
#define TFILE_RBB_METHD             "readByteBuffer"
#define TFILE_GIS_METHD             "getInStream"

#define TBBUF_CLS                   "tachyon/client/TachyonByteBuffer"
#define TBBUF_CLOSE_METHD           "close"

#define TREADT_CLS                  "tachyon/client/ReadType"
#define TWRITET_CLS                 "tachyon/client/WriteType"

#define TISTREAM_CLS               "tachyon/client/InStream"
#define TISTREAM_READ_METHD        "read"
#define TISTREAM_CLOSE_METHD       "close"
#define TISTREAM_SEEK_METHD        "seek"
#define TISTREAM_SKIP_METHD        "skip"

#define TOSTREAM_CLS               "tachyon/client/OutStream"
#define TOSTREAM_WRITE_METHD       "write"
#define TOSTREAM_CLOSE_METHD       "close"
#define TOSTREAM_FLUSH_METHD       "flush"
#define TOSTREAM_CANCEL_METHD      "cancel"

#define BBUF_CLS                    "java/nio/ByteBuffer"
#define BBUF_ALLOC_METHD            "allocate"

namespace tachyon {

enum ReadType {
  NO_CACHE,
  CACHE,
  CACHE_PROMOTE,
};

enum WriteType {
  ASYNC_THROUGH,
  CACHE_THROUGH,
  MUST_CACHE,
  THROUGH,
  TRY_CACHE,
};

class TachyonClient;
class TachyonFile;
class TachyonByteBuffer;

class ByteBuffer;
class InStream;
class OutStream;

typedef TachyonClient* jTachyonClient;
typedef TachyonFile* jTachyonFile;
typedef TachyonByteBuffer* jTachyonByteBuffer;

typedef ByteBuffer* jByteBuffer;
typedef InStream* jInStream;
typedef OutStream* jOutStream;

class JNIObjBase {
  public:
    JNIObjBase(JNIEnv * env, jobject localObj) {
      m_env = env;
      m_obj = env->NewGlobalRef(localObj);
      // this means after the constructor, the localObj will be destroyed
      env->DeleteLocalRef(localObj);
    }

    ~JNIObjBase() {
      m_env->DeleteGlobalRef(m_obj);
    }

  protected:
    JNIEnv *m_env;
    jobject m_obj; // the underlying jobject
};

class TachyonClient : public JNIObjBase {

  public:
    static jTachyonClient createClient(const char *masterUri);
    jTachyonFile getFile(const char *path);
    int createFile(const char *path);

  private:
    // hide constructor, must be instantiated using createClient method
    TachyonClient(JNIEnv *env, jobject tfs) : JNIObjBase(env, tfs) {}

};

class TachyonFile : public JNIObjBase {
  public:
    TachyonFile(JNIEnv *env, jobject tfile) : JNIObjBase(env, tfile){}
    
    long length();
    jTachyonByteBuffer readByteBuffer(int blockIndex);
    jInStream getInStream(ReadType readType);

};

class TachyonByteBuffer : public JNIObjBase {
  public:
    TachyonByteBuffer(JNIEnv *env, jobject tbbuf) : JNIObjBase(env, tbbuf) {}

    jByteBuffer getData();
    void close();

};

class ByteBuffer : public JNIObjBase {
  public:
    ByteBuffer(JNIEnv *env, jobject bbuf): JNIObjBase(env, bbuf){}

    static jByteBuffer allocate(int capacity);
};

class InStream : public JNIObjBase {
  public:
    InStream(JNIEnv *env, jobject istream): JNIObjBase(env, istream){}
  
    void close();
    int read();
    int read(void *buff, int length);
    int read(void *buff, int length, int off, int maxLen);
    void seek(long pos);
    long skip(long n);
};

class OutStream : public JNIObjBase {
  public:
    OutStream(JNIEnv *env, jobject ostream): JNIObjBase(env, ostream){}

    void cancel();
    void close();
    void flush();
    void write(int byte);
    void write(void *buff, int length);
    void write(void *buff, int length, int off, int maxLen);
};


} // namespace tachyon

#ifdef __cplusplus
extern "C" {
#endif

jthrowable enumObjReadType(JNIEnv *env, jobject *objOut, tachyon::ReadType readType);
jthrowable enumObjWriteType(JNIEnv *env, jobject *objOut, tachyon::WriteType writeType);
char* fullTachyonPath(const char *masterUri, const char *filePath);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TACHYON_H_ */

/* vim: set ts=4 sw=4 : */
