/**
 *  @author        Ryan Huang <ryanhuang@cs.ucsd.edu>
 *  @organization  University of California, San Diego
 * 
 * Sample test of Tachyon C/C++ APIs
 *
 */

#include "Tachyon.h"
#include "Util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace tachyon;

const char * program_name;

const char *masterUri;
const char *filePath;

const char *writef = "/hello.txt";

void usage()
{
  fprintf(stderr, "\n\tUsage: %s masterUri testFilePath\n\n", program_name);
}

void testGetFile(jTachyonClient client, const char *fpath)
{
  jTachyonFile nfile = client->getFile(fpath);
  if (nfile == NULL) {
    die("fail to get tachyon file: %s", fpath);
  }
  int fid = client->getFileId(fpath);
  if (fid < 0) {
    die("fail to get file id for %s", fpath);
  }
  long size = nfile->length();
  if (size < 0) {
    die("fail to get tachyon file size");
  }
  printf("got tachyon file, size: %ld, id: %d\n", size, fid);
  if (nfile->isFile() == true) {
    printf("Is a regular file\n");
  } else {
    printf("Not a regular file\n");
  }
  
  char * rpath = nfile->getPath();
  if (rpath == NULL) {
    die("fail to get tachyon file path");
  }

  printf("===================================\n");
  printf("  content for : %s     \n", rpath);
  printf("===================================\n");
  free(rpath);

  jInStream istream = nfile->getInStream(NO_CACHE);
  if (istream == NULL) {
    die("fail to get tachyon file instream");
  }

  char buf[512];
  int rdSz = istream->read(buf, 511);
  while (rdSz > 0) {
    if (rdSz >= 512) {
      printf("impossible read size\n");
      break;
    }
    buf[rdSz] = '\0';
    printf("%s", buf);
    rdSz = istream->read(buf, 511);
  }
  istream->close(); // close istream
  printf("\n");
  printf("===================================\n");
  delete istream;
  delete nfile;
}

void testMkdir(jTachyonClient client, const char *path)
{
  bool ok = client->mkdir(path);
  if (!ok) {
    die("fail to create tachyon dir %s", path);
  }
  printf("created tachyon dir %s\n", path);
}

void testCreateFile(jTachyonClient client, const char *path)
{
  char *rpath;
  int fid = client->createFile(path);
  if (fid < 0) {
    die("fail to create tachyon file %s", path);
  }
  printf("created tachyon file fid:%d\n", fid);
  jTachyonFile nfile = client->getFile(fid);
  if (nfile == NULL) {
    die("fail to get the created file");
  }
  rpath = nfile->getPath();
  if (rpath == NULL) {
    die("fail to get tachyon file path");
  }
  printf("the created tachyon file has path: %s\n", rpath);
  free(rpath);
  delete nfile;
}

void testWriteFile(jTachyonClient client, const char *path)
{
  jTachyonFile nfile = client->getFile(path);
  char content[] = "hello, tachyon!!\n";
  jOutStream ostream = nfile->getOutStream(MUST_CACHE);
  if (ostream == NULL) {
    die("fail to get outstream");
  }
  ostream->write(content, strlen(content));
  ostream->close();
  delete ostream; // release the jobject

  jInStream istream = nfile->getInStream(CACHE);
  char buf[32];
  int rdSz = istream->read(buf, 31);
  buf[rdSz] = '\0';
  printf("Content of the created file:\n");
  printf("%s\n", buf);
  istream->close(); // close istream
  delete istream; // release the jobject
  delete nfile;
}

void testDeleteFile(jTachyonClient client, const char *path, bool recursive)
{
  bool ok = client->deletePath(path, recursive);
  if (!ok) {
    die("fail to delete path %s", path);
  }
  printf("successfully deleted path %s\n", path);
}

void testGetSet(jTachyonClient client)
{
  jTachyonKV kv = TachyonKV::createKV(client, CACHE, MUST_CACHE, 
      DEFAULT_KV_BLOCK_BYTES, "/jkv/");
  if (kv == NULL) {
    die("fail to create tachyon kv\n");
  }
  if (kv->init()) {
    printf("successfully initialized tachyon kv\n");
  } else {
    die("fail to initialize tachyon kv\n");
  }
  char buf[32];
  kv->set("fooo", 4, "bar", 4);
  int sz = kv->get("fooo", 4, buf, 32);
  if (sz < 0) {
    die("fail to get fooo");
  }
  printf("[fooo]=%s\n", buf);
  delete kv;
}

int main(int argc, char*argv[])
{
  program_name = argv[0];
  if (argc != 3) {
    usage();
    exit(1);
  }
  masterUri = argv[1];
  filePath = argv[2];

  try {
    jTachyonClient client = TachyonClient::createClient(masterUri);
    if (client == NULL) {
      die("fail to create tachyon client");
    }
    char * fullFilePath = fullTachyonPath(masterUri, filePath);
    testGetFile(client, fullFilePath);
    testCreateFile(client, writef);
    testWriteFile(client, writef);
    testDeleteFile(client, writef, false);
    testMkdir(client, "/tachyontest");
    testDeleteFile(client, "/tachyontest", true);
    // testGetSet(client);
    delete client;
  } catch (const jni::NativeException &e) {
    e.dump();
  }
  return 0;
}

/* vim: set ts=4 sw=4 : */
