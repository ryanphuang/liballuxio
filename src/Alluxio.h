/**
 *  @author        Adam Storm
 *  @organization  IBM
 * 
 * Alluxio C/C++ APIs
 *
 */

#ifndef __ALLUXIO_H_
#define __ALLUXIO_H_

#include<jni.h>
#include<stdint.h>
#include<chrono>
#include<functional>

#include "JNIHelper.h"

#define BBUF_CLS                    "java/nio/ByteBuffer"

#define TREADT_CLS                  "alluxio/client/ReadType"
#define TWRITET_CLS                 "alluxio/client/WriteType"

// TODO: While these macros have been changed (via search and replace) to
// alluxio, they may not be used correctly yet.  No porting has been done
// to the underlying calling functions.
// InStream
#define TISTREAM_CLS                "alluxio/client/InStream"
#define TFILE_ISTREAM_CLS           "alluxio/client/FileInStream"
#define TBLOCK_ISTREAM_CLS          "alluxio/client/BlockInStream"
#define TLOCAL_BLOCK_ISTREAM_CLS    "alluxio/client/LocalBlockInStream"
#define TREMOTE_BLOCK_ISTREAM_CLS   "alluxio/client/RemoteBlockInStream"
#define TEMPTY_BLOCK_ISTREAM_CLS    "alluxio/client/EmptyBlockInStream"

// OutStream
#define TOSTREAM_CLS                "alluxio/client/OutStream"
#define TFILE_OSTREAM_CLS           "alluxio/client/FileOutStream"
#define TBLOCK_OSTREAM_CLS          "alluxio/client/BlockOutStream"

namespace alluxio {

enum ReadType {
  NO_CACHE,
  CACHE,
  CACHE_PROMOTE,
};

enum WriteType {
  ASYNC_THROUGH,
  CACHE_THROUGH,
  MUST_CACHE,
  NONE,
  THROUGH,
};

class AlluxioFileSystem;
class AlluxioByteBuffer;
class AlluxioCreateFileOptions;
class AlluxioOpenFileOptions;
class AlluxioURI;
class ClientContext;
class Configuration;

class ByteBuffer;
class InStream;
class OutStream;
class FileOutStream;
class FileInStream;

typedef AlluxioCreateFileOptions* jAlluxioCreateFileOptions;
typedef AlluxioOpenFileOptions* jAlluxioOpenFileOptions;
typedef Configuration* jConfiguration;
typedef AlluxioFileSystem* jAlluxioFileSystem;
typedef AlluxioByteBuffer* jAlluxioByteBuffer;
typedef AlluxioURI* jAlluxioURI;

typedef ByteBuffer* jByteBuffer;
typedef InStream* jInStream;
typedef OutStream* jOutStream;
typedef FileOutStream* jFileOutStream;
typedef FileInStream*  jFileInStream;

class JNIObjBase {
  public:
    JNIObjBase(jni::Env env, jobject localObj): m_env(env) {
      m_obj = env->NewGlobalRef(localObj);
      // this means after the constructor, the localObj will be destroyed
      env->DeleteLocalRef(localObj);
    }

    ~JNIObjBase() { m_env->DeleteGlobalRef(m_obj); }

    jobject getJObj() { return m_obj; }
    jni::Env& getEnv() { return m_env; }

  protected:
    jni::Env m_env;
    jobject m_obj; // the underlying jobject
};

class AlluxioClientContext 
{
    // TODO: Clean up this class so that we construct first, then set the
    // host and port after
    public:
        AlluxioClientContext(const char *host, const char *port);

    private:
};

class AlluxioCreateFileOptions : public JNIObjBase
{
    public:
        static jAlluxioCreateFileOptions getCreateFileOptions();
        void setWriteType(WriteType writeType);
        jobject getOptions()
        {
            return m_obj;
        }

    private:
        AlluxioCreateFileOptions(jni::Env env, jobject createFileOptions) :
            JNIObjBase(env, createFileOptions) {}
};


// TODO: To be implemented
class AlluxioRenameFileOptions : public JNIObjBase
{
};

class AlluxioOpenFileOptions : public JNIObjBase
{
    public:
        static jAlluxioOpenFileOptions getOpenFileOptions();
        void setReadType(ReadType readType);
        jobject getOptions()
        {
            return m_obj;
        }

    private:
        AlluxioOpenFileOptions(jni::Env env, jobject openFileOptions) :
            JNIObjBase(env, openFileOptions) {}
};

class AlluxioFileSystem : public JNIObjBase {

  public:
    static jAlluxioFileSystem getFileSystem(AlluxioClientContext *acc);
    static jAlluxioFileSystem copyClient(jAlluxioFileSystem client);

    bool exists(const char *path);

    jFileOutStream createFile(const char *path);
    jFileOutStream createFile(const char *path, AlluxioCreateFileOptions *options);
    jFileInStream openFile(const char *path);
    jFileInStream openFile(const char *path, AlluxioOpenFileOptions *options);
    void renameFile(const char *origPath, const char *newPath);

    void renameFile(const char *origPath, const char *newPath, 
            AlluxioRenameFileOptions *options)
    {
        // TODO: To be implemented
        throw std::bad_function_call();
    }

    void appendToFile(const char *path, void *buff, int length);
    void appendToFile(const char *path, void *buff, int length, 
            AlluxioCreateFileOptions *options);
    void createDirectory(const char *path);

    void deletePath(const char *path);
    void deletePath(const char *path, bool recursive);

  private:
    // hide constructor, must be instantiated using getFileSystem method
    AlluxioFileSystem(jni::Env env, jobject tfs, AlluxioClientContext *acc) : 
        JNIObjBase(env, tfs),
        clientContext(acc) {}

    AlluxioClientContext *clientContext;
};

class AlluxioByteBuffer : public JNIObjBase {
  public:
    AlluxioByteBuffer(jni::Env env, jobject tbbuf) : JNIObjBase(env, tbbuf) {}

    jByteBuffer getData();
    void close();

};

class ByteBuffer : public JNIObjBase {
  public:
    ByteBuffer(jni::Env env, jobject bbuf): JNIObjBase(env, bbuf){}

    static jByteBuffer allocate(int capacity);
};

class InStream : public JNIObjBase {
  public:
    InStream(jni::Env env, jobject istream): JNIObjBase(env, istream){}
  
    void close();
    int read();
    int read(void *buff, int length);
    int read(void *buff, int length, int off, int maxLen, 
          bool measureTime,
          std::chrono::duration<double>* pBufferCreationTimeCounter,
          std::chrono::duration<double>* pReadTimeCounter,
          std::chrono::duration<double>* pBufferCopyTimeCounter);
    void seek(long pos);
    long skip(long n);
  
};

class FileInStream : public InStream 
{
   public:
      FileInStream(jni::Env env, jobject fileInStream) : 
         InStream (env, fileInStream){}
};

class EmptyBlockInStream : public InStream {
};

class BlockInStream : public InStream {
};

class LocalBlockInStream : public BlockInStream {
};

class RemoteBlockInStream : public BlockInStream {
};

class OutStream : public JNIObjBase {
  public:
    OutStream(jni::Env env, jobject ostream): JNIObjBase(env, ostream){}

    void cancel();
    void close();
    void flush();
    void write(int byte);
    void write(const void *buff, int length);
    void write(const void *buff, int length, int off, int maxLen);

};

class FileOutStream : public OutStream 
{
  public: 
    FileOutStream(jni::Env env, jobject fileOutStream) : 
        OutStream (env, fileOutStream){}
};

class BlockOutStream : public OutStream { 
    bool canWrite(); //TODO
    long getBlockId(); //TODO
    long getBlockOffset(); //TODO
    long getRemainingSpaceByte(); //TODO
};

class AlluxioURI : public JNIObjBase {
  public:
    static jAlluxioURI newURI(const char *pathStr);
    static jAlluxioURI newURI(const char *scheme, const char *authority, const char *path);
    static jAlluxioURI newURI(jAlluxioURI parent, jAlluxioURI child);

    AlluxioURI(jni::Env env, jobject uri): JNIObjBase(env, uri){}
};



} // namespace alluxio

#ifdef __cplusplus
extern "C" {
#endif

jobject enumObjReadType(alluxio::jni::Env& env, alluxio::ReadType readType);
jobject enumObjWriteType(alluxio::jni::Env& env, alluxio::WriteType writeType);

char* fullAlluxioPath(const char *masterUri, const char *filePath);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ALLUXIO_H_ */

/* vim: set ts=4 sw=4 : */
