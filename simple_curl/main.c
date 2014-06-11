/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2011, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
/* Example code that uploads a file name 'foo' to a remote script that accepts
 * "HTML form based" (as described in RFC1738) uploads using HTTP POST.
 *
 * The imaginary form we'll fill in looks like:
 *
 * <form method="post" enctype="multipart/form-data" action="examplepost.cgi">
 * Enter file: <input type="file" name="sendfile" size="40">
 * Enter file name: <input type="text" name="filename" size="30">
 * <input type="submit" value="send" name="submit">
 * </form>
 *
 * This exact source code has not been verified to work.
 */

#include <stdio.h>
#include <string.h>

#include <curl/curl.h>

#define MAX_PATH_LEN 100

typedef struct writeData_t {
  char destPath[MAX_PATH_LEN];
  FILE *fd;
  //rsComm_t someIRODS stuff
} writeData_t;

typedef struct readData_t {
  char sourcePath[MAX_PATH_LEN];
  FILE *fd;
  long offset;
} readData_t;

static size_t write_callback(void *buffer, size_t size, size_t nmemb, void* userp)
{
  struct writeData_t *writeData = (struct writeData_t *)userp;

  if (!writeData) {
    return -11;
  }

  if (! writeData->fd) {
    writeData->fd = fopen(writeData->destPath, "w+");
  }

  return fwrite(buffer, size, nmemb, writeData->fd);
}
  
static size_t read_callback(void *buffer, size_t size, size_t nmemb, void* userp)
{
  struct readData_t *readData = (struct readData_t *)userp;

  if (!readData) {
    return -11;
  }

  if (! readData->fd) {
    readData->fd = fopen(readData->sourcePath, "r");
  }
  
  return fread(buffer, size, nmemb, readData->fd);
}
  
int main(int argc, char *argv[])
{
  CURL *curl;
  CURLcode res;

  char *destPath = "/tmp/y18.png";
  char *sourcePath = "y18.gif";

  writeData_t writeData;
  readData_t readData;

  struct curl_httppost *formpost=NULL;
  struct curl_httppost *lastptr=NULL;

  curl_global_init(CURL_GLOBAL_ALL);

  /* Fill in the filename field */
  curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "output_format",
               CURLFORM_COPYCONTENTS, "png",
               CURLFORM_END);
  
  /* Fill in the filename field */
  curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "file",
               CURLFORM_FILENAME, sourcePath, //sourcePath
               CURLFORM_STREAM, &readData,
               CURLFORM_CONTENTSLENGTH, 100,
               CURLFORM_CONTENTTYPE, "application/octet-stream",
               CURLFORM_END);

  //TODO: zero out writedata
  
  snprintf(readData.sourcePath, MAX_PATH_LEN, "%s", sourcePath);
  readData.fd = 0;
  
  snprintf(writeData.destPath, MAX_PATH_LEN, "%s", destPath);
  writeData.fd = 0;
  
  curl = curl_easy_init();
  if(curl) {
    /* what URL that receives this POST */
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:5000/");
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, &read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &readData);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeData);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);

    /* then cleanup the formpost chain */
    curl_formfree(formpost);

    // close source file
    if (writeData.fd) {
      fclose(writeData.fd);
    }
  }
  return 0;
}


