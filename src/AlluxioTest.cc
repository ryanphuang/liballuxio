/**
 *  @author        Adam Storm <ajstorm@ca.ibm.com>
 *  @organization  IBM
 * 
 * Sample test of Alluxio C/C++ APIs
 *
 */

#include "Alluxio.h"
#include "Util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//FIXME: This is using a mix of C and C++ IO right now.  Convert to all
// C++
#include <fstream>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <functional>

using namespace alluxio;

const char * program_name;

const char *masterUri;
const char *filePath;

const char *gFileToCreate = "/hello.txt";
const char *gDirToCreate = "/alluxiotest";
const char *gPathSeparatorString = "/";
const char gPathSeparatorChar = '/';

void usage()
{
   std::cerr << "Usage: " << program_name << " masterHost masterPort [file]" 
      << std::endl;
}

char* inputFileToAlluxioPath (char* file)
{
    const char *fileName;
    // Find the file name in the supplied path by looking for the last separator
    fileName = strrchr(file, gPathSeparatorChar);
    if (fileName == NULL)
    {
        // Handle the case where the supplied path is local to the directory
        fileName = file;
    }
    else
    {
        // move past the last separator since we're explicitly adding it below
        fileName++;
    }

    // FIXME: Change to std::vector
    char * alluxioPath = (char *) calloc(strlen(fileName) + 
            strlen(gPathSeparatorString) + strlen(gDirToCreate) + 1, 1);

    if (!alluxioPath)
    {
        throw std::bad_alloc();
    }

    strncpy(alluxioPath, gDirToCreate, strlen(gDirToCreate));
    strncpy(&alluxioPath[strlen(gDirToCreate)], gPathSeparatorString, 
            strlen(gPathSeparatorString));
    strncpy(&alluxioPath[strlen(gDirToCreate) + strlen(gPathSeparatorString)], 
            fileName, strlen(fileName));

    return alluxioPath;
}

void testCreateDirectory(jAlluxioFileSystem client, const char *path)
{
  client->createDirectory(path);
  std::cout << "created alluxio dir " << path << std::endl;
}

jFileOutStream testCreateFileWithOptions(jAlluxioFileSystem client, const char *path)
{
  jFileOutStream fileOutStream;
  bool fileExists = false;
  AlluxioCreateFileOptions* options = AlluxioCreateFileOptions::getCreateFileOptions();

  options->setWriteType(CACHE_THROUGH);

  fileOutStream = client->createFile(path, options);

  printf("created alluxio file:%s\n", path);

  return fileOutStream;
}

jFileOutStream testCreateFile(jAlluxioFileSystem client, const char *path)
{
  jFileOutStream fileOutStream;
  bool fileExists = false;

  fileOutStream = client->createFile(path);

  if (fileOutStream == NULL) 
  {
    printf("failed to create alluxio file %s\n", path);
    goto exit;
  }
  else
  {
     printf("created alluxio file:%s\n", path);
  }

exit:

  return fileOutStream;
}

void testReadLargeFile(jAlluxioFileSystem client, jFileInStream fileInStream, 
      const char* path, char* inputBuffer, int bufferSize)
{
  std::chrono::duration<double> duration = std::chrono::duration<double>::zero();
  std::chrono::duration<double> elapsedTime = std::chrono::duration<double>::zero();
  std::chrono::duration<double> bufferCreationTime = std::chrono::duration<double>::zero();
  std::chrono::duration<double> alluxioReadTime = std::chrono::duration<double>::zero();
  std::chrono::duration<double> bufferCopyTime = std::chrono::duration<double>::zero();
  std::chrono::time_point<std::chrono::system_clock> startTime, stopTime;
  AlluxioOpenFileOptions* openOptions = AlluxioOpenFileOptions::getOpenFileOptions();
  int bytesRead = bufferSize;
  const bool measureTime = true;

  openOptions->setReadType(CACHE_PROMOTE);

  // Now do the reading from Alluxio
  startTime = std::chrono::system_clock::now();
  fileInStream = client->openFile(path, openOptions);
  stopTime = std::chrono::system_clock::now();
  duration = stopTime - startTime;
  std::cout << "Opened file " << path << " from Alluxio in " << duration.count() 
     << " seconds" << std::endl;

  if (fileInStream == NULL) 
  {
     printf("failed to open alluxio file %s\n", path);
     goto exit;
  }
  else
  {
     printf("successfully opened file:%s for reading\n", path);
  }

  elapsedTime = std::chrono::duration<double>::zero();

  while (bytesRead == bufferSize)
  {
     startTime = std::chrono::system_clock::now();
     bytesRead = fileInStream->read(
           inputBuffer, bufferSize, 0, bufferSize, measureTime,
           &bufferCreationTime, &alluxioReadTime, &bufferCopyTime);
     stopTime = std::chrono::system_clock::now();
     elapsedTime += stopTime - startTime;
  }

  std::cout << bufferCreationTime.count() 
     << " seconds creating buffers" << std::endl << alluxioReadTime.count() << 
     " seconds reading buffers" << std::endl << bufferCopyTime.count() << 
     " seconds gaining access to buffers"<< std::endl;

  fileInStream->close();

  std::cout << "Read complete" << std::endl;
  std::cout << "Spent " << elapsedTime.count() << " seconds reading from Alluxio" << std::endl;
  delete (fileInStream); 

exit:
  return;
}

void testCopyFile(jAlluxioFileSystem client, const char *inPath, const char *alluxioPath)
{
  const char *fileNameInInpath;
  jFileOutStream targetOutStream;
  jFileInStream  fileInStream;
  bool fileExists = false;
  std::ifstream inputFile;
  char *inputBuffer;
  const int bufferSize = 1000000;
  int bytesRead = bufferSize;
  AlluxioCreateFileOptions* createOptions = NULL;
  AlluxioOpenFileOptions* openOptions = NULL;
  std::chrono::duration<double> duration = std::chrono::duration<double>::zero();
  std::chrono::duration<double> elapsedTimeReading = std::chrono::duration<double>::zero();
  std::chrono::duration<double> elapsedTimeWriting = std::chrono::duration<double>::zero();
  std::chrono::time_point<std::chrono::system_clock> startTime, stopTime;

  std::cout.precision(15);

  createOptions = AlluxioCreateFileOptions::getCreateFileOptions();

  createOptions->setWriteType(CACHE_THROUGH);

  targetOutStream = client->createFile(alluxioPath, createOptions);

  startTime = std::chrono::system_clock::now();
  inputFile.open(inPath, std::ios::in | std::ios::binary);
  stopTime = std::chrono::system_clock::now();
  duration = stopTime - startTime;
  std::cout << "Opened file " << inPath << " on disk in " << duration.count() 
     << " seconds" << std::endl;

  // FIXME: Change to std::vector
  inputBuffer = (char *) calloc(bufferSize, 1);

  if (!inputBuffer)
  {
      throw std::bad_alloc();
  }

  while (bytesRead == bufferSize)
  {
     startTime = std::chrono::system_clock::now();

     inputFile.read(inputBuffer, bufferSize);

     stopTime = std::chrono::system_clock::now();

     duration = stopTime - startTime;

     elapsedTimeReading += duration;

     bytesRead = inputFile.gcount();

     startTime = std::chrono::system_clock::now();

     targetOutStream->write(inputBuffer, bytesRead);

     stopTime = std::chrono::system_clock::now();

     duration = stopTime - startTime;

     elapsedTimeWriting += duration;
  }

  startTime = std::chrono::system_clock::now();
  targetOutStream->close();
  stopTime = std::chrono::system_clock::now();
  elapsedTimeWriting += stopTime - startTime;

  std::cout << "File copy complete" << std::endl;
  std::cout << "Spent " << elapsedTimeReading.count() 
     << " seconds reading from disk" << std::endl;
  std::cout << "Spent " << elapsedTimeWriting.count() 
     << " seconds writing to Alluxio" << std::endl;
  delete(targetOutStream);

  // Now do the reading from Alluxio
  testReadLargeFile(client, fileInStream, alluxioPath, inputBuffer, bufferSize);

  free(inputBuffer);

  return;
}

void testAppendFile(jAlluxioFileSystem client, char* path, char* appendString)
{
    // TODO: This is all common code (from testCopyFile).  It should be encapsulated
    client->appendToFile(path, appendString, strlen(appendString));

    std::cout << "Successfully appended: \n\"" << appendString << "\" to file " <<
        path << std::endl;
}

void testWriteFile(jFileOutStream fileOutStream)
{
  char content[] = "hello, alluxio!!\n";
  fileOutStream->write(content, strlen(content));
  printf("Successfully wrote %s to file\n", content);

}

jFileInStream testOpenFile(jAlluxioFileSystem client, const char *path)
{
   char *rpath;
   jFileInStream fileInStream;

   fileInStream = client->openFile(path);

   printf("Successfully opened file:%s\n", path);

exit:

   return fileInStream;
}

void testReadFile(jFileInStream fileInStream)
{
  char buf[32];
  int rdSz = fileInStream->read(buf, 31);
  buf[rdSz] = '\0';
  printf("Content of the created file:\n");
  printf("%s\n", buf);
}

void testDeleteFile(jAlluxioFileSystem client, const char *path, bool recursive)
{
  client->deletePath(path, recursive);
  printf("successfully deleted path %s\n", path);
}


int main(int argc, char*argv[])
{
  program_name = argv[0];
  if (argc < 3 || argc > 4) {
    usage();
    exit(1);
  }

  // FIXME error checking of input params
  char *host = argv[1];
  char *port = argv[2];
  char *file = NULL;
  char appendString[] = "Successfully appended to file\n";

  if (argc == 4)
  {
     file = argv[3];
  }

  try {
      AlluxioClientContext acc (host, port);
      jAlluxioFileSystem client = AlluxioFileSystem::getFileSystem(&acc);
      if (client == NULL) {
          die("fail to create alluxio client");
      }
      
      // Test the creation of a file with the default options
      jFileOutStream fileOutStream = testCreateFile(client, gFileToCreate);

      // Close and delete created file
      fileOutStream->close();
      delete fileOutStream;
      testDeleteFile(client, gFileToCreate, false);

      // Test the creation of a file with non-default options
      fileOutStream = testCreateFileWithOptions(client, gFileToCreate);

      // Write to newly created file
      testWriteFile(fileOutStream);

      fileOutStream->close();
      delete fileOutStream;

      // Open file for reading
      jFileInStream fileInStream = testOpenFile(client, gFileToCreate);

      // Read from file
      testReadFile(fileInStream);
      fileInStream->close(); 
      delete fileInStream; 

      // Test file deletion
      testDeleteFile(client, gFileToCreate, false);

      // Test path deletion
      testDeleteFile(client, gDirToCreate, true);

      // Create a new directory
      testCreateDirectory(client, gDirToCreate);

      if (file != NULL)
      {
          char* alluxioFile = inputFileToAlluxioPath(file);

          // Copy file (if specified) to new directory
          testCopyFile(client, file, alluxioFile);

          // Append to file
          testAppendFile(client, alluxioFile, appendString);

          free(alluxioFile);
      }

      // Cleanup
      delete client;

  } catch (const jni::NativeException &e) {
    e.dump();
  }
  return 0;
}
