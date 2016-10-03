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
#include <memory>
#include <vector>

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
  MUST_CACHE,
  TRY_CACHE, // Deprecated
  CACHE_THROUGH,
  THROUGH,
  ASYNC_THROUGH,
  NONE,
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

//typedef ClientContext* jClientContext;
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


class JNIStringBase {
  public:
    JNIStringBase(jni::Env env, jstring localString): m_env(env) { 
      m_string = reinterpret_cast<jstring>(env->NewGlobalRef(localString));
      // this means after the constructor, the localObj will be destroyed
      env->DeleteLocalRef(localString);
    }

    ~JNIStringBase() { m_env->DeleteGlobalRef(m_string); }

    jstring getJString() { return m_string; }
    jni::Env& getEnv() { return m_env; }

  protected:
    jni::Env m_env;
    jstring m_string; // the underlying jstring
};

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
    public:
        AlluxioClientContext();
        ~AlluxioClientContext();
        static void connect(const char *host, const char *port, 
                const char *accessKey, const char *secretKey);
        static void setAlluxioStringConstant(jni::Env &env, const char *key, const char *value);

        jobject getJObj() { return m_baseFileSystem; }
        jni::Env &getEnv() { return m_env; }

    private:
        bool m_mustDetachInDtor;
        bool m_mustDeleteLocalRef;
        jni::Env m_env;
        /// Pointer to Java object that has all APIs for Alluxio file system.
        jobject m_baseFileSystem;

        void attach();
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

/// Enum to control what is filtered in the listPath call
enum class ListPathFilter {
    /// No filtering in listPath().  Return everything under given path.
    NONE,
    /// Only return directories in listPath() call
    DIRECTORIES_ONLY
};

/**
   Abstraction layer to alluxio file system. 
*/
class AlluxioFileSystem {
    public:
        AlluxioFileSystem(AlluxioClientContext& clientContext);
        bool exists(const char *path);
        long int fileSize(const char *path);
        void createDirectory(const char *path);
        void deletePath(const char *path, bool recursive = false);

        jFileInStream openFile(const char *path, AlluxioOpenFileOptions *options = nullptr);
        jFileOutStream createFile(const char * path, AlluxioCreateFileOptions *options = nullptr);
        jFileOutStream openFileForAppend(const char *path, AlluxioCreateFileOptions *options);
        void completeAppend(const char *path, jFileOutStream fileOutStream);
        void renameFile(const char *origPath, const char *newPath);
        std::vector<std::string> listPath(const char * path, ListPathFilter filter);

    private:
        AlluxioClientContext& mClient;
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
