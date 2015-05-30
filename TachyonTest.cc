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

// replace the address with actual address
const char *masterAddr = "tachyon://localhost:19998";
const char *filePath = "tachyon://localhost:19998/README.txt/README.txt";

int main()
{
  jTachyonClient client = TachyonClient::createClient(masterAddr);
  if (client == NULL) {
    die("fail to create tachyon client\n");
  }
  jTachyonFile file = client->getFile(filePath);
  if (file == NULL) {
    die("fail to get tachyon file\n");
  }
  long size = file->length();
  printf("got tachyon file, size: %ld\n", size);
  return 0;
}

/* vim: set ts=4 sw=4 : */
