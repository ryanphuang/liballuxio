/**
 *  @author        Ryan Huang <ryanhuang@cs.ucsd.edu>
 *  @organization  University of California, San Diego
 * 
 * Tachyon C/C++ APIs
 *
 */

#include "Tachyon.h"
#include "JNIHelper.h"

TachyonClient createClient(const char *masterHost, const char *masterPort)
{
  JNIEnv *env = getJNIEnv();
  if (env == NULL) {
    return NULL;
  }

  return NULL;
}

int createFile(TachyonClient client, const char * path)
{

  return 0;
}

/* vim: set ts=4 sw=4 : */
