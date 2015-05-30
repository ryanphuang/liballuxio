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

class TachyonClient;
class TachyonFile;

typedef TachyonClient* jTachyonClient;
typedef TachyonFile* jTachyonFile;

class TachyonClient {

  public:
    static jTachyonClient createClient(const char *masterUri);
    jTachyonFile getFile(const char *path);
    int createFile(const char *path);

  private:
    // hide constructor, must be instantiated using createClient method
    TachyonClient(jobject tfs) : m_tfs(tfs) {}

    // the underlying jobect to interact with tfs
    jobject m_tfs;
};

class TachyonFile {
  public:
    TachyonFile(jobject tfile) : m_tfile(tfile){}
    
    long length();

  private:

    // the underlying jobect to interact with file
    jobject m_tfile;
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
