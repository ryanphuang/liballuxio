/**
 *  @author        Ryan Huang <ryanhuang@cs.ucsd.edu>
 *  @organization  University of California, San Diego
 * 
 * Sample test of Tachyon C/C++ APIs
 *
 */

#include "Tachyon.h"

#include <stdio.h>

int main()
{
  TachyonClient client = createClient("localhost:19998");
  if (client == NULL) {
    printf("fail to create tachyon client\n");
  }
  return 0;
}

/* vim: set ts=4 sw=4 : */
