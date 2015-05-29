#ifndef __TACHYON_H_
#define __TACHYON_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TachyonFS {

} TachyonFS;

typedef TachyonFS* TachyonClient;

TachyonClient createClient(const char *masterHost, const char *masterPort);
int createFile(TachyonClient client, const char * path);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TACHYON_H_ */

/* vim: set ts=4 sw=4 : */
