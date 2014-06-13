
// =-=-=-=-=-=-=-
#include "apiHeaderAll.hpp"
#include "msParam.hpp"
#include "reGlobalsExtern.hpp"
#include "irods_ms_plugin.hpp"


// =-=-=-=-=-=-=-
// STL Includes
#include <string>
#include <iostream>


// =-=-=-=-=-=-=-
// cURL Includes
#include <curl/curl.h>
#include <curl/easy.h>



typedef struct writeData_t {
    char path[MAX_NAME_LEN];
    int desc;
    rsComm_t *rsComm;
} writeData_t;


typedef struct readData_t {
  char path[MAX_NAME_LEN];
  int desc;
  rsComm_t *rsComm;
} readData_t;


class irodsCurl {
private:
    // iRODS serv, char *destPath er handle
    rsComm_t *rsComm;

    // cURL handle
    CURL *curl;
 

public:
    irodsCurl( rsComm_t *comm ) {
        rsComm = comm;

        curl = curl_easy_init();
        if ( !curl ) {
            rodsLog( LOG_ERROR, "irodsCurl: %s", curl_easy_strerror( CURLE_FAILED_INIT ) );
        }
    }

    ~irodsCurl() {
        if ( curl ) {
            curl_easy_cleanup( curl );
        }
    }

    //Gets file name from a path
    char *getName (const char *path) {

	char tmpPath[strlen(path)]; //FIXME: does tmpPath do anything? why do we have this?
      
	strcpy(tmpPath, path);

        char* p;

        char* outStr;
        outStr = (char*)malloc(strlen(path));  //FIXME: should this malloc one extra byte for the null terminator?

        p = strrchr (tmpPath, '/'); 

       	snprintf(outStr, strlen(path), "%s", p +1);

	return outStr;
}


    int get( char *url, char *sourcePath, char *destPath, char *ext) {

	CURL *curl;
	CURLcode res = CURLE_OK;

        readData_t readData;
        openedDataObjInp_t openedSource;

        writeData_t writeData;	                // the "file descriptor" for our destination object
        openedDataObjInp_t openedTarget;	// for closing iRODS object after writing

        int status;

	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;

	curl_global_init(CURL_GLOBAL_ALL);

        /* Fill in the file upload field */
        curl_formadd(&formpost,
                &lastptr,
                CURLFORM_COPYNAME, "output_format",
                CURLFORM_COPYCONTENTS, ext,
                CURLFORM_END);

        /* Fill in the filename field */
        curl_formadd(&formpost,
                &lastptr,
                CURLFORM_COPYNAME, "file",
                CURLFORM_FILENAME, getName(sourcePath), 
                CURLFORM_STREAM, &readData,
                CURLFORM_CONTENTSLENGTH, 100,  //This needs to be the size of the upload
                CURLFORM_CONTENTTYPE, "application/octet-stream",
                CURLFORM_END);

        // Zero fill openedDataObjInp
        memset( &openedTarget, 0, sizeof( openedDataObjInp_t ) );

        // Set up writeData
        snprintf(writeData.path, MAX_NAME_LEN, "%s", destPath);
        writeData.desc = 0;	                 // the object is yet to be created
        writeData.rsComm = rsComm;

        // Set up writeDataInp
        snprintf(readData.path, MAX_NAME_LEN, "%s", sourcePath);
        readData.desc = 0;	// the object is yet to be created
	readData.rsComm = rsComm;

        // Set up easy handler
 	curl = curl_easy_init();

	/* what URL that receives this POST */
        curl_easy_setopt(curl, CURLOPT_URL, url);

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, &irodsCurl::my_read_obj);
        curl_easy_setopt(curl, CURLOPT_READDATA, &readData);
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &irodsCurl::my_write_obj);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeData);

        // CURL call
        res = curl_easy_perform( curl );

        // Some error logging
        if ( res != CURLE_OK ) {
            rodsLog( LOG_ERROR, "irodsCurl::get: cURL error: %s", curl_easy_strerror( res ) );
        }

        // close iRODS objects
        if (writeData.desc) {
            openedTarget.l1descInx = writeData.desc;
            status = rsDataObjClose(rsComm, &openedTarget);
            if (status < 0) {
                rodsLog(LOG_ERROR, "irodsCurl::get: rsDataObjClose failed for %s, status = %d",
                         writeData.path, status);
            }
        }
        if (readData.desc) {
            openedSource.l1descInx = readData.desc;
            status = rsDataObjClose(rsComm, &openedSource);
            if (status < 0) {
                rodsLog(LOG_ERROR, "irodsCurl::get: rsDataObjClose failed for %s, status = %d",
                         readData.path, status);
            }
        }
        
        /* then cleanup the formpost chain */
        curl_formfree(formpost);

        return res;
    }

    static size_t my_read_obj(void *buffer, size_t size, size_t nmemb, void* userp) {
        struct readData_t *readData = (struct readData_t *) userp;

        dataObjInp_t file;
        openedDataObjInp_t openedFile;
        bytesBuf_t bytesBuf;
        
        size_t bytesRead;
        
        //Make sure we have something to read from
        if (!readData) {
            rodsLog(LOG_ERROR, "my_read_obj: readData is NULL, status = %d", SYS_INTERNAL_NULL_INPUT_ERR);
            return SYS_INTERNAL_NULL_INPUT_ERR;
        }

        //Zero fill input structs
        memset(&file, 0, sizeof (dataObjInp_t));
        memset(&openedFile, 0, sizeof (openedDataObjInp_t));
        memset(&bytesBuf, 0, sizeof (bytesBuf_t));

        //If this is the first call we need to create our data object before writing to it
        if (!readData->desc) {
            strncpy(file.objPath, readData->path, MAX_NAME_LEN);

            readData->desc = rsDataObjOpen(readData->rsComm, &file);
            if (readData->desc < 0) { //TODO: <= 2 instead of < 0? Look up rsDataObjOpen return codes
                rodsLog(LOG_ERROR, "my_read_obj: PROBLEM OPENING DATA OBJECT. Status =  %d", readData->desc);
                return readData->desc;
            }
        }

        //Setup buffer for rsDataObjRead
        bytesBuf.len = (int)(size * nmemb);
        bytesBuf.buf = buffer;

        //Setup input struct for rsDataObjRead
	openedFile.l1descInx = readData->desc;
        openedFile.len = bytesBuf.len;
        
        bytesRead = rsDataObjRead(readData->rsComm, &openedFile, &bytesBuf);
        if (bytesRead < 0) {
            rodsLog(LOG_ERROR, "my_read_obj: PROBLEM READING FILE. Status =  %d", bytesRead);
            return bytesRead;
        }
        return (bytesRead);
    }

    // Custom callback function for the curl handler, to write to an iRODS object
    static size_t my_write_obj(void *buffer, size_t size, size_t nmemb, writeData_t *writeData) {
        dataObjInp_t file;	// input struct for rsDataObjCreate
        openedDataObjInp_t openedFile;	// input struct for rsDataObjWrite
        bytesBuf_t bytesBuf;	// input buffer for rsDataObjWrite
        size_t written;	// return value

        // Make sure we have something to write to
        if (!writeData) {
            rodsLog( LOG_ERROR, "my_write_obj: writeData is NULL, status = %d", SYS_INTERNAL_NULL_INPUT_ERR );
            return SYS_INTERNAL_NULL_INPUT_ERR;
        }

        // Zero fill input structs
        memset(&file, 0, sizeof(dataObjInp_t));
        memset(&openedFile, 0, sizeof(openedDataObjInp_t));
        memset(&bytesBuf, 0, sizeof(bytesBuf_t));


        // If this is the first call we need to create our data object before writing to it
        if (!writeData->desc) {
            strncpy(file.objPath, writeData->path, MAX_NAME_LEN);

            // Overwrite existing file (for this tutorial only, in case the example has been run before)
            addKeyVal(&file.condInput, FORCE_FLAG_KW, "");  //TODO: this should be an error, not an overwrite

            writeData->desc = rsDataObjCreate(writeData->rsComm, &file);

            // No create?
            if ( writeData->desc <= 2 ) {
                rodsLog( LOG_ERROR, "my_write_obj: rsDataObjCreate failed for %s, status = %d", file.objPath, writeData->desc);
                return (writeData->desc);
            }
        }


        // Set up input buffer for rsDataObjWrite
        bytesBuf.len = (int)(size * nmemb);
        bytesBuf.buf = buffer;

        // Set up input struct for rsDataObjWrite
	openedFile.l1descInx = writeData->desc;
        openedFile.len = bytesBuf.len;

        // Write to data object
        written = rsDataObjWrite(writeData->rsComm, &openedFile, &bytesBuf);
        
        //TODO: should we be handling error cases when written < 0? Check return values of rsDataObjWrite

        return (written);
    }

}; // class irodsCurl


extern "C" {

    int irods_curl_get( msParam_t* url, msParam_t* source_obj, msParam_t* ext_obj,
                                        msParam_t* dest_obj, ruleExecInfo_t* rei ) {
        dataObjInp_t destObjInp, *myDestObjInp;	/* for parsing input object */

        // Sanity checks
        if ( !rei || !rei->rsComm ) {
            rodsLog( LOG_ERROR, "irods_curl_get: Input rei or rsComm is NULL." );
            return ( SYS_INTERNAL_NULL_INPUT_ERR );
        }

	// get destination path from sourcePath and exten.
	char *sourceStr = parseMspForStr(source_obj);
	char *extStr = parseMspForStr(ext_obj);

	//char *source = "/tempZone/home/public/new.png";

	char tmpSource[strlen(sourceStr) + 10];

	strcpy(tmpSource, sourceStr);


    	char *lastdot = strrchr (tmpSource, '.');
	char *lastsep = ('/' == 0) ? NULL : strrchr (tmpSource, '/');

	if (lastdot != NULL)
	{
        // and it's before the extenstion separator.

        	if (lastsep != NULL)
		{
            		if (lastsep < lastdot)
			{
                	// then remove it.

                	*lastdot = '\0';
            		}
        	}
		else
		{
            	// Has extension separator with no path separator.

            	*lastdot = '\0';
        	}
    	}
        char destStr[strlen(sourceStr)+10];

	snprintf(destStr, strlen(sourceStr) + 10, "%s%s%s", tmpSource, ".", extStr);


	fillStrInMsParam(dest_obj, destStr);


        // Get path of destination object
        rei->status = parseMspForDataObjInp( dest_obj, &destObjInp, &myDestObjInp, 0 );
        if ( rei->status < 0 ) {
            rodsLog( LOG_ERROR, "irods_curl_get: Input object error. status = %d", rei->status );
            return ( rei->status );
        }

        // Create irodsCurl instance
        irodsCurl myCurl( rei->rsComm );

        // Call irodsCurl::get

        rei->status = myCurl.get( parseMspForStr( url ), sourceStr,  destObjInp.objPath, extStr);

        // Done
        return rei->status;

    }


// =-=-=-=-=-=-=-
// 2. Create the plugin factory function which will return a microservice
// table entry
    irods::ms_table_entry* plugin_factory() {
        // =-=-=-=-=-=-=-
        // 3. allocate a microservice plugin which takes the number of function
        // params as a parameter to the constructor
        irods::ms_table_entry* msvc = new irods::ms_table_entry( 4 );

        // =-=-=-=-=-=-=-
        // 4. add the microservice function as an operation to the plugin
        // the first param is the name / key of the operation, the second
        // is the name of the function which will be the microservice
        msvc->add_operation( "irods_curl_get", "irods_curl_get" );

        // =-=-=-=-=-=-=-
        // 5. return the newly created microservice plugin
        return msvc;
    }
}	// extern "C"
