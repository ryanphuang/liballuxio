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


#define TFILE_CLS                   "tachyon/client/TachyonFile"
#define TFILE_LENGTH_METHD          "length"
#define TFILE_RBB_METHD             "readByteBuffer"

class ByteBuffer;
class TachyonClient;
class TachyonFile;
class TachyonByteBuffer;

typedef ByteBuffer* jByteBuffer;

typedef TachyonClient* jTachyonClient;
typedef TachyonFile* jTachyonFile;
typedef TachyonByteBuffer* jTachyonByteBuffer;

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

};



#ifdef __cplusplus
extern "C" {
#endif

char* fullTachyonPath(const char *masterUri, const char *filePath);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TACHYON_H_ */

/* vim: set ts=4 sw=4 : */
