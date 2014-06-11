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



typedef struct writeDataInp_t {
    char objPath[MAX_NAME_LEN];
    int l1descInx;
    rsComm_t *rsComm;
} writeDataInp_t;


typedef struct readData_t {
  char sourcePath[MAX_NAME_LEN];
  FILE *fd;
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

    int get( char *url, char *sourcePath, char *destPath, char *ext) {

	CURL *curl;
	CURLcode res = CURLE_OK;
        writeDataInp_t writeDataInp;	// the "file descriptor" for our destination object
        openedDataObjInp_t openedDataObjInp;	// for closing iRODS object after writing
        
        readData_t readData;
        
        int status;

	FILE *fd;
	fd = fopen(sourcePath, "rb"); /* open file to upload */
	if(!fd) { return 1; /* can't continue */ }

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
                CURLFORM_FILENAME, "y18.gif", //This needs to be the filename of the upload
                CURLFORM_STREAM, &readData,
                CURLFORM_CONTENTSLENGTH, 100,  //This needs to be the size of the upload
                CURLFORM_CONTENTTYPE, "application/octet-stream",
                CURLFORM_END);

        // Zero fill openedDataObjInp
        memset( &openedDataObjInp, 0, sizeof( openedDataObjInp_t ) );

        // Set up writeDataInp
        snprintf( writeDataInp.objPath, MAX_NAME_LEN, "%s", destPath );
        writeDataInp.l1descInx = 0;	// the object is yet to be created
        writeDataInp.rsComm = rsComm;

        // Set up writeDataInp
        snprintf(readData.sourcePath, MAX_NAME_LEN, "%s", sourcePath);
        readData.fd = 0;	// the object is yet to be created

        // Set up easy handler
 	curl = curl_easy_init();


	/* what URL that receives this POST */
        curl_easy_setopt(curl, CURLOPT_URL, url);

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, &irodsCurl::my_read_obj);
        curl_easy_setopt(curl, CURLOPT_READDATA, &readData);
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &irodsCurl::my_write_obj);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeDataInp );

        // CURL call
        res = curl_easy_perform( curl );

        // Some error logging
        if ( res != CURLE_OK ) {
            rodsLog( LOG_ERROR, "irodsCurl::get: cURL error: %s", curl_easy_strerror( res ) );
        }

        // close iRODS object
        if ( writeDataInp.l1descInx ) {
            openedDataObjInp.l1descInx = writeDataInp.l1descInx;
            status = rsDataObjClose( rsComm, &openedDataObjInp );
            if ( status < 0 ) {
                rodsLog( LOG_ERROR, "irodsCurl::get: rsDataObjClose failed for %s, status = %d",
                         writeDataInp.objPath, status );
            }
        }
        
        /* then cleanup the formpost chain */
        curl_formfree(formpost);

        return res;
    }

    static size_t my_read_obj(void *buffer, size_t size, size_t nmemb, void* userp) {
        struct readData_t *readData = (struct readData_t *) userp;

        if (!readData) {
            return -11;
        }

        if (!readData->fd) {
            readData->fd = fopen(readData->sourcePath, "r");
        }

        return fread(buffer, size, nmemb, readData->fd);
    }

    // Custom callback function for the curl handler, to write to an iRODS object
    static size_t my_write_obj( void *buffer, size_t size, size_t nmemb, writeDataInp_t *writeDataInp ) {
        dataObjInp_t dataObjInp;	// input struct for rsDataObjCreate
        openedDataObjInp_t openedDataObjInp;	// input struct for rsDataObjWrite
        bytesBuf_t bytesBuf;	// input buffer for rsDataObjWrite
        size_t written;	// return value

        int l1descInx;


        // Make sure we have something to write to
        if ( !writeDataInp ) {
            rodsLog( LOG_ERROR, "my_write_obj: writeDataInp is NULL, status = %d", SYS_INTERNAL_NULL_INPUT_ERR );
            return SYS_INTERNAL_NULL_INPUT_ERR;
        }

        // Zero fill input structs
        memset( &dataObjInp, 0, sizeof( dataObjInp_t ) );
        memset( &openedDataObjInp, 0, sizeof( openedDataObjInp_t ) );


        // If this is the first call we need to create our data object before writing to it
        if ( !writeDataInp->l1descInx ) {
            strncpy( dataObjInp.objPath, writeDataInp->objPath, MAX_NAME_LEN );

            // Overwrite existing file (for this tutorial only, in case the example has been run before)
            addKeyVal( &dataObjInp.condInput, FORCE_FLAG_KW, "" );

            writeDataInp->l1descInx = rsDataObjCreate( writeDataInp->rsComm, &dataObjInp );


            // No create?
            if ( writeDataInp->l1descInx <= 2 ) {
                rodsLog( LOG_ERROR, "my_write_obj: rsDataObjCreate failed for %s, status = %d", dataObjInp.objPath, writeDataInp->l1descInx );
                return ( writeDataInp->l1descInx );
            }
        }


        // Set up input buffer for rsDataObjWrite
        bytesBuf.len = ( int )( size * nmemb );
        bytesBuf.buf = buffer;


        // Set up input struct for rsDataObjWrite
        openedDataObjInp.l1descInx = writeDataInp->l1descInx;;
        openedDataObjInp.len = bytesBuf.len;


        // Write to data object
        written = rsDataObjWrite( writeDataInp->rsComm, &openedDataObjInp, &bytesBuf );

        return ( written );
    }

}; // class irodsCurl


extern "C" {

// =-=-=-=-=-=-=-
// 1. Write a standard issue microservice
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

	char *source = "/tempZone/home/public/new.png";

	char tmpSource[strlen(source) + 10];

	strcpy(tmpSource, source);


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
        char destStr[strlen(source)+10];

	snprintf(destStr, strlen(source) + 10, "%s%s%s", tmpSource, ".", extStr);


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


