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

using namespace tachyon;

const char *masterUri;
const char *filePath;

const char * program_name;

void usage()
{
  fprintf(stderr, "\n\tUsage: %s masterUri testFilePath\n\n", program_name);
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

  char * fullFilePath = fullTachyonPath(masterUri, filePath);

  printf("full file path: %s\n", fullFilePath);

  jTachyonClient client = TachyonClient::createClient(masterUri);
  if (client == NULL) {
    die("fail to create tachyon client\n");
  }
  jTachyonFile file = client->getFile(fullFilePath);
  if (file == NULL) {
    die("fail to get tachyon file\n");
  }
  long size = file->length();
  printf("got tachyon file, size: %ld\n", size);
  return 0;
}

/* vim: set ts=4 sw=4 : */
