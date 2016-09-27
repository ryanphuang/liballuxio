/**
 *  @author        Adam Storm <ajstorm@ca.ibm.com>
 *  @organization  IBM
 * 
 * Alluxio C/C++ APIs
 *
 */

#include "Alluxio.h"
#include "Util.h"

#include <string>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>

using namespace alluxio;
using namespace alluxio::jni;

/**
   Constructor for AlluxioClientContext

   This will attach to the JVM thread created for this process.  Prior to
   calling this AlluxioClientContext::connect() must be called to cause the JVM
   thread to connect to the master node.
*/
AlluxioClientContext::AlluxioClientContext()
    : m_mustDetachInDtor(false), m_mustDeleteLocalRef(false) {
  attach();
}

/**
   Destructor.
*/
AlluxioClientContext::~AlluxioClientContext() {
    if (m_mustDeleteLocalRef) {
        m_env.deleteLocalRef(m_baseFileSystem);
        m_mustDeleteLocalRef = false;
    }
    if (m_mustDetachInDtor) {
        m_env.DetachCurrentThread();
        m_mustDetachInDtor = false;
    }
}

/**
  Set a string value in the Alluxio constants

  @param[in] env The jni environment
  @param[in] key Key of string constant to be set
  @param[in] value Value to be assigned
*/
void AlluxioClientContext::setAlluxioStringConstant(jni::Env &env, const char *key, const char *value) {
  jvalue  retSet;

  // Get the key object
  JNIObjBase jKeyObject(env, env.getEnumObject("alluxio/Constants", key, "Ljava/lang/String;"));

  // Construct the string
  JNIStringBase jValueString(env, env.newStringUTF(value, value));

  // Call the methods to set the string
  env.callStaticMethod(&retSet, "alluxio/Configuration", "set",
                      "(Ljava/lang/String;Ljava/lang/String;)V", 
                      static_cast<jstring>(jKeyObject.getJObj()),
                      jValueString.getJString());
}

/**
  Connect to the Alluxio master node.

  This only needs to be done once per process.  However, each thread that
  interface with Alluxio needs to perform an attach first before it can use
  JNI.

  @param[in] host Host name of the Alluxio master node
  @param[in] port Port name for the Alluxio master node
*/
void AlluxioClientContext::connect(const char *host, const char *port, 
        const char *accessKey, const char *secretKey) {
  Env env;
  jvalue retClientContext;
  jvalue retSet;

  AlluxioClientContext::setAlluxioStringConstant(env, "MASTER_HOSTNAME", host);
  AlluxioClientContext::setAlluxioStringConstant(env, "MASTER_RPC_PORT", port);

  // Setup credentials for AWS S3A
  AlluxioClientContext::setAlluxioStringConstant(env, "S3A_ACCESS_KEY", accessKey);
  AlluxioClientContext::setAlluxioStringConstant(env, "S3A_SECRET_KEY", secretKey);

  // Init the client context
  env.callStaticMethod(&retClientContext, "alluxio/client/ClientContext",
                       "init", "()V");
}

/**
   Attach the current thread to a running JVM.

   This can only be used if the process had previously called connect() in
   same/other thread.
*/
void AlluxioClientContext::attach() {
  // We only attach if not already done so.  This gives us nested style of
  // semantics if we run a bunch of APIs that continually need to attach.  A
  // detach is only done once we tear down the client context of when the
  // thread first attached.
  if (m_env.GetIsThreadDetached() == true) {
    m_env.AttachCurrentThread();
    m_mustDetachInDtor = true;
  }

  // Get a pointer to the alluxio java object for use in subsequent Java calls.
  jvalue ret;
  m_env.callStaticMethod(&ret, "alluxio/client/file/BaseFileSystem", "get",
                         "()Lalluxio/client/file/BaseFileSystem;");
  m_baseFileSystem = ret.l;
  m_mustDeleteLocalRef = true;
}

jAlluxioCreateFileOptions AlluxioCreateFileOptions::getCreateFileOptions()
{
    Env env;
    jvalue jFileOptions;

    // First get the ClientContext.Configuration
    env.callStaticMethod(&jFileOptions, 
            "alluxio/client/file/options/CreateFileOptions", "defaults", 
            "()Lalluxio/client/file/options/CreateFileOptions;");

    return new AlluxioCreateFileOptions(env, jFileOptions.l);
}

void AlluxioCreateFileOptions::setWriteType(WriteType writeType)
{
    jobject jWriteType;
    jvalue  ret;

    // Get the enum value for the specified write type
    jWriteType = enumObjWriteType(m_env, writeType);
    
    // Call the method to set the write type
    m_env.callMethod(&ret, m_obj, "setWriteType", 
            "(Lalluxio/client/WriteType;)Lalluxio/client/file/options/CreateFileOptions;",
            jWriteType);
}

jAlluxioOpenFileOptions AlluxioOpenFileOptions::getOpenFileOptions()
{
    Env env;
    jvalue jFileOptions;

    // First get the ClientContext.Configuration
    env.callStaticMethod(&jFileOptions, 
            "alluxio/client/file/options/OpenFileOptions", "defaults", 
            "()Lalluxio/client/file/options/OpenFileOptions;");

    return new AlluxioOpenFileOptions(env, jFileOptions.l);
}

void AlluxioOpenFileOptions::setReadType(ReadType readType)
{
    jobject jReadType;
    jvalue  ret;

    // Get the enum value for the specified write type
    jReadType = enumObjReadType(m_env, readType);
    
    // Call the method to set the write type
    m_env.callMethod(&ret, m_obj, "setReadType", 
            "(Lalluxio/client/ReadType;)Lalluxio/client/file/options/OpenFileOptions;",
            jReadType);
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

// TODO: Template this funtion for time measurement?
int InStream::read(void *buff, int length, int off, int maxLen, 
      bool measureTime = false,
      std::chrono::duration<double>* pBufferCreationTimeCounter = NULL,
      std::chrono::duration<double>* pReadTimeCounter = NULL,
      std::chrono::duration<double>* pBufferCopyTimeCounter = NULL)
{
   jbyteArray jBuf;
   jvalue ret;
   int rdSz;

   std::chrono::duration<double> duration = std::chrono::duration<double>::zero();
   std::chrono::time_point<std::chrono::system_clock> startTime, stopTime;

   try {

      if (measureTime)
      {
         startTime = std::chrono::system_clock::now();
      }

      jBuf = m_env.newByteArray(length);

      if (measureTime)
      {
         stopTime = std::chrono::system_clock::now();
         duration = stopTime - startTime;
         *pBufferCreationTimeCounter += duration;
      }

      if (off < 0 || maxLen <= 0 || length == maxLen)
      {
         if (measureTime)
         {
            startTime = std::chrono::system_clock::now();
         }

         m_env.callMethod(&ret, m_obj, "read", "([B)I", jBuf);

         if (measureTime)
         {
            stopTime = std::chrono::system_clock::now();
         }
      }
      else
      {
         if (measureTime)
         {
            startTime = std::chrono::system_clock::now();
         }

         m_env.callMethod(&ret, m_obj, "read", "([BII)I", jBuf, off, maxLen);

         if (measureTime)
         {
            stopTime = std::chrono::system_clock::now();
         }
      }
      if (measureTime)
      {
         duration = stopTime - startTime;
         *pReadTimeCounter += duration;
      }
   } catch (NativeException) {
      if (jBuf != NULL) {
         m_env->DeleteLocalRef(jBuf);
      }
      throw;
   }
   rdSz = ret.i;
   if (rdSz > 0) {
      if (measureTime)
      {
         startTime = std::chrono::system_clock::now();
      }
      m_env->GetByteArrayRegion(jBuf, 0, rdSz, (jbyte*) buff);
      // TODO: It's much more efficient to get direct access to the buffer,
      // but doing so requires the caller to create the java buffer.
      // Create a read method that allows for this option.
      // buff = m_env->GetDirectBufferAddress(jBuf);
      if (measureTime)
      {
         stopTime = std::chrono::system_clock::now();
         duration = stopTime - startTime;
         *pBufferCopyTimeCounter += duration;
      }
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
  m_env.callMethod(&ret, m_obj, "skip", "(J)J", (jlong) n);
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

  std::unique_ptr<char[]> jbuff(new char[length * sizeof(char)]);
  m_env->GetByteArrayRegion(jBuf, 0, length, (jbyte*) jbuff.get());
  // printf("byte array in write: %s\n", jbuff);

  if (off < 0 || maxLen <= 0 || length == maxLen)
    m_env.callMethod(NULL, m_obj, "write", "([B)V", jBuf);
  else
    m_env.callMethod(NULL, m_obj, "write", "([BII)V", jBuf, (jint) off, (jint) maxLen);
  m_env->DeleteLocalRef(jBuf);
}

// Call the templates
void OutStream::close()
{
  m_env.callMethod(NULL, m_obj, "close", "()V");
}

//TODO: These methods are not yet tested yet.  Use with care
void OutStream::cancel() 
{
  m_env.callMethod(NULL, m_obj, "cancel", "()V");
}

void OutStream::flush()
{
  m_env.callMethod(NULL, m_obj, "flush", "()V");
}

//////////////////////////////////////////
// AlluxioURI
//////////////////////////////////////////

jAlluxioURI AlluxioURI::newURI(const char *pathStr)
{
  Env env;
  jobject retObj;
  jstring jPathStr;
  
  jPathStr = env.newStringUTF(pathStr, "path");
  retObj = env.newObject("alluxio/AlluxioURI", "(Ljava/lang/String;)V", jPathStr);
  env->DeleteLocalRef(jPathStr);
  return new AlluxioURI(env, retObj);
}

jAlluxioURI AlluxioURI::newURI(const char *scheme, const char *authority, const char *path)
{
  Env env;
  jobject retObj;
  jstring jscheme, jauthority, jpath;

  jscheme = env.newStringUTF(scheme, "scheme");
  jauthority = env.newStringUTF(authority, "authority");
  jpath = env.newStringUTF(path, "path");
  retObj = env.newObject("alluxio/AlluxioURI",
                  "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", 
                  jscheme, jauthority, jpath);
  env->DeleteLocalRef(jscheme);
  env->DeleteLocalRef(jauthority);
  env->DeleteLocalRef(jpath);
  return new AlluxioURI(env, retObj);
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
  return env.getEnumObject(TREADT_CLS, valueName, "Lalluxio/client/ReadType;");
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
    default:
          throw std::runtime_error("invalid writeType");
  }
  return env.getEnumObject(TWRITET_CLS, valueName, "Lalluxio/client/WriteType;");
}

//////////////////////////////////////////
// AlluxioFileSystem
//////////////////////////////////////////

/**
  Constructor

  @param[in] clientContext Context that has either connected to alluxio, or
             attached to the running JVM that has connected.
*/
AlluxioFileSystem::AlluxioFileSystem(AlluxioClientContext &clientContext)
    : mClient(clientContext) {}

bool AlluxioFileSystem::exists(const char *path) {
  jvalue ret;

  std::unique_ptr<AlluxioURI> uri(AlluxioURI::newURI(path));

  mClient.getEnv().callMethod(&ret, mClient.getJObj(), "exists",
                              "(Lalluxio/AlluxioURI;)Z", uri->getJObj());

  return ret.z;
}

void AlluxioFileSystem::createDirectory(const char *path) {
  jvalue ret;
  std::unique_ptr<AlluxioURI> uri(AlluxioURI::newURI(path));
  mClient.getEnv().callMethod(&ret, mClient.getJObj(), "createDirectory",
                              "(Lalluxio/AlluxioURI;)V", uri->getJObj());
  return;
}

/**
  Delete a file or directory in Alluxio

  @param[in] path Path name to delete.  Can be a file or a directory.
  @param[in] recursive If the path to be deleted is a directory, the flag
             specifies whether the directory content should be recursively
             deleted as well
*/
void AlluxioFileSystem::deletePath(const char *path, bool recursive) {
  jvalue ret;

  std::unique_ptr<AlluxioURI> uri(AlluxioURI::newURI(path));

  if (!recursive) {
    mClient.getEnv().callMethod(&ret, mClient.getJObj(), "delete",
                                "(Lalluxio/AlluxioURI;)V", uri->getJObj());
  } else {
    jvalue deleteOptionsDefaults;
    jvalue deleteOptionsSetRecursive;

    mClient.getEnv().callStaticMethod(
        &deleteOptionsDefaults, "alluxio/client/file/options/DeleteOptions",
        "defaults", "()Lalluxio/client/file/options/DeleteOptions;");

    mClient.getEnv().callMethod(
        &deleteOptionsSetRecursive, (jobject)deleteOptionsDefaults.l,
        "setRecursive", "(Z)Lalluxio/client/file/options/DeleteOptions;",
        (jboolean)recursive);

    mClient.getEnv().callMethod(
        &ret, mClient.getJObj(), "delete",
        "(Lalluxio/AlluxioURI;Lalluxio/client/file/options/DeleteOptions;)V",
        uri->getJObj(), (jobject)deleteOptionsSetRecursive.l);
  }
}

jFileInStream AlluxioFileSystem::openFile(const char *path,
                                          AlluxioOpenFileOptions *options) {
  jvalue ret;

  std::unique_ptr<AlluxioURI> uri(AlluxioURI::newURI(path));

  jFileInStream fileInStream = NULL;

  if (options == NULL) {
    mClient.getEnv().callMethod(
        &ret, mClient.getJObj(), "openFile",
        "(Lalluxio/AlluxioURI;)Lalluxio/client/file/FileInStream;",
        uri->getJObj());
  } else {
    mClient.getEnv().callMethod(&ret, mClient.getJObj(), "openFile",
                                "(Lalluxio/AlluxioURI;Lalluxio/client/file/"
                                "options/OpenFileOptions;)Lalluxio/client/file/"
                                "FileInStream;",
                                uri->getJObj(), options->getOptions());
  }

  // FIXME: Change to shared_ptr?
  return (new FileInStream(mClient.getEnv(), ret.l));
}

jFileOutStream
AlluxioFileSystem::createFile(const char *path,
                              AlluxioCreateFileOptions *options) {
  jvalue ret;

  std::unique_ptr<AlluxioURI> uri(AlluxioURI::newURI(path));

  if (options == NULL) {
    mClient.getEnv().callMethod(
        &ret, mClient.getJObj(), "createFile",
        "(Lalluxio/AlluxioURI;)Lalluxio/client/file/FileOutStream;",
        uri->getJObj());
  } else {
    mClient.getEnv().callMethod(&ret, mClient.getJObj(), "createFile",
                                "(Lalluxio/AlluxioURI;Lalluxio/client/file/"
                                "options/CreateFileOptions;)Lalluxio/client/"
                                "file/FileOutStream;",
                                uri->getJObj(), options->getOptions());
  }

  return (new FileOutStream(mClient.getEnv(), ret.l));
}

void AlluxioFileSystem::renameFile(const char *origPath, const char *newPath) {
  jvalue ret;

  std::unique_ptr<AlluxioURI> origURI(AlluxioURI::newURI(origPath));
  std::unique_ptr<AlluxioURI> newURI(AlluxioURI::newURI(newPath));

  mClient.getEnv().callMethod(&ret, mClient.getJObj(), "rename",
                              "(Lalluxio/AlluxioURI;Lalluxio/AlluxioURI;)V",
                              origURI->getJObj(), newURI->getJObj());
}

// FIXME: We should be able to query the open file options and not require them
// to be passed in
jFileOutStream
AlluxioFileSystem::openFileForAppend(const char *path,
                                     AlluxioCreateFileOptions *options) {
  const int bufferSize = 1000000;
  const char *appendSuffix = ".append";
  int bytesRead = bufferSize;
  jFileInStream origFile = NULL;
  jFileOutStream newFile = NULL;
  // FIXME: Change to std::vector
  char *inputBuffer = (char *)calloc(bufferSize, 1);

  if (inputBuffer == NULL) {
    throw std::bad_alloc();
  }

  int appendPathLength = strlen(path) + strlen(appendSuffix) + 1;

  // FIXME: Change to std::vector
  char *appendPath = (char *)malloc(appendPathLength);

  if (appendPath == NULL) {
    throw std::bad_alloc();
  }

  strncpy(appendPath, path, appendPathLength);
  strcat(appendPath, ".append");

  try {
    origFile = openFile(path, nullptr);
    newFile = createFile(appendPath, options);

    // Copy the original file over to the new file
    while (bytesRead == bufferSize) {
      bytesRead = origFile->read(inputBuffer, bufferSize);
      if (bytesRead > 0) {
          newFile->write(inputBuffer, bytesRead);
      }
    }

    origFile->close();
  }
  catch (...) {
    free(inputBuffer);
    free(appendPath);
    throw;
  }

  free(inputBuffer);
  free(appendPath);

  return newFile;
}

void AlluxioFileSystem::completeAppend(const char *path,
                                       jFileOutStream fileOutStream) {
  const char *appendSuffix = ".append";
  int appendPathLength = strlen(path) + strlen(appendSuffix) + 1;
  char *appendPath = (char *)malloc(appendPathLength);

  if (appendPath == NULL) {
    throw std::bad_alloc();
  }

  strncpy(appendPath, path, appendPathLength);
  strcat(appendPath, ".append");

  try {
    fileOutStream->close();

    // Delete original file
    deletePath(path);

    // NOTE: This is the critical section of this method.  If a failure
    // occurs here, we will be left with a .append file and no original file.

    // TODO: we should add some defensive code in the open file path to
    // perform this rename automatically if we only find a .append file

    // Rename file
    renameFile(appendPath, path);
  }
  catch (...) {
    free(appendPath);
    throw;
  }

  free(appendPath);
}

long int AlluxioFileSystem::fileSize(const char *path) {
  jvalue retGetStatus;
  jvalue retGetSize;

  // FIXME: where are we cleaning up ret* memory?  Is there a huge memory leak
  // going on here?  Also, this whole file doesn't appear to be exception safe.
  std::unique_ptr<AlluxioURI> uri(AlluxioURI::newURI(path));

  mClient.getEnv().callMethod(
      &retGetStatus, mClient.getJObj(), "getStatus",
      "(Lalluxio/AlluxioURI;)Lalluxio/client/file/URIStatus;", uri->getJObj());

  mClient.getEnv().callMethod(&retGetSize, retGetStatus.l, "getLength", "()J");

  return retGetSize.j;
}

/**
   Return a list of files in the given path.

   @param[in] path Path to list files/dirs from
   @param[in] filter Type of filter to apply to list call
   @return Vector of strings.  Each entry is a file in the path.
*/
std::vector<std::string> AlluxioFileSystem::listPath(const char *path,
                                                     ListPathFilter filter) {
  std::vector<std::string> files;

  jvalue retList;
  std::unique_ptr<AlluxioURI> uri(AlluxioURI::newURI(path));
  mClient.getEnv().callMethod(&retList, mClient.getJObj(), "listStatus",
                              "(Lalluxio/AlluxioURI;)Ljava/util/List;", uri->getJObj());

  // List<URIStatus>.size()
  jvalue retGetSize;
  mClient.getEnv().callMethod(&retGetSize, retList.l, "size", "()I");

  files.reserve(retGetSize.i);
  for(int i = 0; i < retGetSize.i; i++) {
    jvalue uriObj;
    mClient.getEnv().callMethod(&uriObj, retList.l, "get",
                                "(I)Ljava/lang/Object;", (jint)i);

    jvalue uriString;
    mClient.getEnv().callMethod(&uriString, uriObj.l, "toString",
            "()Ljava/lang/String;", uriObj.l);
    std::string rawObjStr;
    mClient.getEnv().jstringToString(static_cast<jstring>(uriString.l), rawObjStr);

    // Parse out just the file name from the raw string
    const static std::string PATH_MARKER_START = "path=";
    const static std::string PATH_MARKER_END = ", ";
    std::string pathStr = rawObjStr.substr(rawObjStr.find(PATH_MARKER_START) + PATH_MARKER_START.size());
    pathStr = pathStr.substr(0, pathStr.find(PATH_MARKER_END));

    switch (filter) {
    case ListPathFilter::NONE:
      files.push_back(pathStr);
      break;

    case ListPathFilter::DIRECTORIES_ONLY:
      if (rawObjStr.find("folder=true") != std::string::npos) {
        files.push_back(pathStr);
      }
      break;

    default:
      std::ostringstream errMsg;
      errMsg << "Unknown listPath() filter " << int(filter);
      throw std::runtime_error(errMsg.str());
    }
  }

  return files;
}


/* vim: set ts=4 sw=4 : */
