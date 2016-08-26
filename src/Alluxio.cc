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

AlluxioClientContext::AlluxioClientContext(const char *host, const char *port)
{
    Env env;
    jvalue  retClientContext;
    jvalue  retConfiguration;
    jvalue  retSet;
    jstring jHostString;
    jstring jPortString;
    jobject jHostKeyString;
    jobject jPortKeyString;

    // First get the ClientContext.Configuration
    env.callStaticMethod(&retConfiguration, 
            "alluxio/client/ClientContext", "getConf", "()Lalluxio/Configuration;");

    // Get the hostname and port strings from the alluxio.Constants class
    jHostKeyString = env.getEnumObject(
          "alluxio/Constants", "MASTER_HOSTNAME", "Ljava/lang/String;");
    jPortKeyString = env.getEnumObject(
          "alluxio/Constants", "MASTER_RPC_PORT", "Ljava/lang/String;"); 

    jHostString = env.newStringUTF(host, "host");
    jPortString = env.newStringUTF(port, "port");

    // Call the methods to set the hostname and port
    env.callMethod(&retSet, retConfiguration.l, "set",
            "(Ljava/lang/String;Ljava/lang/String;)V", jHostKeyString, jHostString);
    env.callMethod(&retSet, retConfiguration.l, "set",
            "(Ljava/lang/String;Ljava/lang/String;)V", jPortKeyString, jPortString);

    // Init the client context
    env.callStaticMethod(&retClientContext, 
          "alluxio/client/ClientContext", "init", "()V");

    // Cleanup jvalues
    env->DeleteLocalRef(jHostString);
    env->DeleteLocalRef(jHostKeyString);
    env->DeleteLocalRef(jPortString);
    env->DeleteLocalRef(jPortKeyString);
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

jAlluxioFileSystem AlluxioFileSystem::getFileSystem(AlluxioClientContext *acc)
{
  Env env;
  jvalue ret;

  // might throw exception, caller needs to handle it
  env.callStaticMethod(&ret, "alluxio/client/file/BaseFileSystem", "get", 
                "()Lalluxio/client/file/BaseFileSystem;");
  
  return new AlluxioFileSystem(env, ret.l, acc);
}

jAlluxioFileSystem AlluxioFileSystem::copyClient(jAlluxioFileSystem client)
{
  return new AlluxioFileSystem(client->getEnv(), client->getJObj(), 
          client->clientContext);
}


void AlluxioFileSystem::deletePath(const char * path)
{
  jvalue ret;

  jAlluxioURI uri = AlluxioURI::newURI(path);

  m_env.callMethod(&ret, m_obj, "delete", "(Lalluxio/AlluxioURI;)V",
                   uri->getJObj());

  delete uri;
  return;
}

bool AlluxioFileSystem::exists(const char * path)
{
  jvalue ret;

  jAlluxioURI uri = AlluxioURI::newURI(path);

  m_env.callMethod(&ret, m_obj, "exists", "(Lalluxio/AlluxioURI;)Z",
                   uri->getJObj());

  delete uri;
  return ret.z;
}

jFileOutStream AlluxioFileSystem::createFile(const char * path, 
        AlluxioCreateFileOptions *options)
{
    jvalue ret;

    jAlluxioURI uri = AlluxioURI::newURI(path);

    jFileOutStream fileOutStream = NULL;

    if (uri == NULL)
    {
       goto exit;
    }

    m_env.callMethod(&ret, m_obj, "createFile", 
            "(Lalluxio/AlluxioURI;Lalluxio/client/file/options/CreateFileOptions;)Lalluxio/client/file/FileOutStream;",
            uri->getJObj(), options->getOptions());

    delete uri;

    // FIXME: Change to shared_ptr
    fileOutStream = new FileOutStream(m_env, ret.l); 

exit:
    return fileOutStream;
}

jFileOutStream AlluxioFileSystem::createFile(const char * path)
{
    jvalue ret;

    jAlluxioURI uri = AlluxioURI::newURI(path);

    jFileOutStream fileOutStream = NULL;

    if (uri == NULL)
    {
       goto exit;
    }

    m_env.callMethod(&ret, m_obj, "createFile", 
            "(Lalluxio/AlluxioURI;)Lalluxio/client/file/FileOutStream;",
            uri->getJObj());

    delete uri;

    // FIXME: Change to shared_ptr
    fileOutStream = new FileOutStream(m_env, ret.l); 

exit:
    return fileOutStream;
}

jFileInStream AlluxioFileSystem::openFile(const char * path)
{
    jvalue ret;

    jAlluxioURI uri = AlluxioURI::newURI(path);

    jFileInStream fileInStream = NULL;

    if (uri == NULL)
    {
       goto exit;
    }

    m_env.callMethod(&ret, m_obj, "openFile", 
            "(Lalluxio/AlluxioURI;)Lalluxio/client/file/FileInStream;",
            uri->getJObj());

    delete uri;
    // FIXME: Change to shared_ptr?
    fileInStream = new FileInStream(m_env, ret.l); 

exit:
    return fileInStream;
}

jFileInStream AlluxioFileSystem::openFile(const char * path, AlluxioOpenFileOptions *options)
{
    jvalue ret;

    jAlluxioURI uri = AlluxioURI::newURI(path);

    jFileInStream fileInStream = NULL;

    if (uri == NULL)
    {
       goto exit;
    }

    m_env.callMethod(&ret, m_obj, "openFile", 
            "(Lalluxio/AlluxioURI;Lalluxio/client/file/options/OpenFileOptions;)Lalluxio/client/file/FileInStream;",
            uri->getJObj(), options->getOptions());

    delete uri;

    // FIXME: Change to shared_ptr?
    fileInStream = new FileInStream(m_env, ret.l); 

exit:
    return fileInStream;
}

// FIXME: Rename so that deletePath and createDirectory have matching naming?
void AlluxioFileSystem::createDirectory(const char *path)
{
   jvalue ret;
   jAlluxioURI uri = AlluxioURI::newURI(path);

   m_env.callMethod(&ret, m_obj, "createDirectory", 
         "(Lalluxio/AlluxioURI;)V", uri->getJObj());
   delete uri;

exit:
   return;
}

void AlluxioFileSystem::deletePath(const char * path, bool recursive)
{
  jvalue ret;

  jAlluxioURI uri = AlluxioURI::newURI(path);

  if (!recursive)
  {
     m_env.callMethod(&ret, m_obj, "delete", "(Lalluxio/AlluxioURI;)V",
                      uri->getJObj());
  }
  else
  {
     jvalue deleteOptionsDefaults;
     jvalue deleteOptionsSetRecursive;

     m_env.callStaticMethod(&deleteOptionsDefaults, 
           "alluxio/client/file/options/DeleteOptions", "defaults", 
           "()Lalluxio/client/file/options/DeleteOptions;");

     m_env.callMethod(&deleteOptionsSetRecursive, (jobject) deleteOptionsDefaults.l, 
           "setRecursive", "(Z)Lalluxio/client/file/options/DeleteOptions;", 
           (jboolean) recursive);

     try
     {
        m_env.callMethod(&ret, m_obj, "delete", 
              "(Lalluxio/AlluxioURI;Lalluxio/client/file/options/DeleteOptions;)V",
              uri->getJObj(), (jobject) deleteOptionsSetRecursive.l);
     }
     catch (NativeException &e) 
     {
        std::string exceptionString;
        jni::JavaThrowable *jthrow = e.detail();
        jthrowable throwable = jthrow->getException();
        m_env.throwableToString(throwable, exceptionString);
        std::cout << "Caught exception: " << exceptionString << std::endl;
        goto exit;
     }
  }

exit:

  delete uri;
  return;
}

void AlluxioFileSystem::renameFile(const char *origPath, const char *newPath)
{
    jvalue ret;

    jAlluxioURI origURI = AlluxioURI::newURI(origPath);
    jAlluxioURI newURI = AlluxioURI::newURI(newPath);

    m_env.callMethod(&ret, m_obj, "rename", 
            "(Lalluxio/AlluxioURI;Lalluxio/AlluxioURI;)V", 
            origURI->getJObj(), newURI->getJObj());

    delete origURI;
    delete newURI;
}

void AlluxioFileSystem::appendToFile(const char *path, void *buff, int length)
{
    const int bufferSize = 1000000;
    const char* appendSuffix = ".append";
    int bytesRead = bufferSize;
    // FIXME: Change to std::vector
    char* inputBuffer = (char *) calloc(bufferSize, 1);
    int appendPathLength = strlen(path) + strlen(appendSuffix) + 1;

    // FIXME: Change to std::vector
    char* appendPath = (char *) malloc(appendPathLength);

    strncpy(appendPath, path, appendPathLength);
    strcat(appendPath,".append");

    if (inputBuffer == NULL || appendPath == NULL)
    {
        throw std::bad_alloc();
    }

    jFileInStream origFile = openFile(path);

    // FIXME: Create file should preserve file options of original file
    jFileOutStream newFile = createFile(appendPath);

    // Copy the original file over to the new file
    while (bytesRead == bufferSize)
    {
        bytesRead = origFile->read(inputBuffer, bufferSize);

        newFile->write(inputBuffer, bytesRead);
    }

    // Perform the append
    newFile->write(buff, length);
    origFile->close();
    newFile->close();

    // Delete original file
    deletePath(path);

    // NOTE: This is the critical section of this method.  If a failure
    // occurs here, we will be left with a .append file and no original file.
    
    // TODO: we should add some defensive code in the open file path to
    // perform this rename automatically if we only find a .append file
    
    // Rename file
    renameFile(appendPath, path);

    free(inputBuffer);
    free(appendPath);
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

//TODO: These methods are not yet tested yet.  Use with care
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

  // FIXME: Change to std::vector
  char *jbuff = (char *) malloc(length * sizeof(char));
  m_env->GetByteArrayRegion(jBuf, 0, length, (jbyte*) jbuff);

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


/* vim: set ts=4 sw=4 : */
