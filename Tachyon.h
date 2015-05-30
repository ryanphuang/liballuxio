/**
 *  @author        Ryan Huang <ryanhuang@cs.ucsd.edu>
 *  @organization  University of California, San Diego
 * 
 * Tachyon C/C++ APIs
 *
 */

#ifndef __TACHYON_H_
#define __TACHYON_H_

#ifdef __cplusplus
extern "C" {
#endif

#define TFS_CLS               "tachyon/client/TachyonFS"
#define TFS_GET_NAME          "get"

typedef struct TachyonFS {
} TachyonFS;

typedef TachyonFS* TachyonClient;

TachyonClient createClient(const char *masterAddr);
int createFile(TachyonClient client, const char * path);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TACHYON_H_ */

/* vim: set ts=4 sw=4 : */
