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


#include <stdio.h>

extern int rsDataObjWrite (rsComm_t *rsComm, 
               openedDataObjInp_t *dataObjWriteInp,
               bytesBuf_t *dataObjWriteInpBBuf);
extern int rsDataObjRead (rsComm_t *rsComm, 
               openedDataObjInp_t *dataObjReadInp,
               bytesBuf_t *dataObjReadOutBBuf); 
extern int rsDataObjOpen (rsComm_t *rsComm, dataObjInp_t *dataObjInp);
extern int rsDataObjClose (rsComm_t *rsComm, openedDataObjInp_t *dataObjCloseInp);
extern int rsDataObjCreate (rsComm_t *rsComm, dataObjInp_t *dataObjInp);


typedef struct {
	char objPath[MAX_NAME_LEN];
	int l1descInx;
	rsComm_t *rsComm;
} dataObjHTTPInp_t;


const int MAX_SIZE = 4000000;
const int MAX_OUT_SIZE = 40000;

typedef struct {
    char data_array[MAX_SIZE];
    int offset;
} dataStringURLsObject_t;

typedef struct {
    char objPath[MAX_NAME_LEN];
    int l1descInx;
    rsComm_t *rsComm;
} writeDataInp_t;



class irodsCurl {
private:
    // iRODS server handle
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

    int get( char *url, char *objPath ) {
        CURLcode res = CURLE_OK;
        writeDataInp_t writeDataInp;	// the "file descriptor" for our destination object
        openedDataObjInp_t openedDataObjInp;	// for closing iRODS object after writing
        int status;

        // Zero fill openedDataObjInp
        memset( &openedDataObjInp, 0, sizeof( openedDataObjInp_t ) );

        // Set up writeDataInp
        snprintf( writeDataInp.objPath, MAX_NAME_LEN, "%s", objPath );
        writeDataInp.l1descInx = 0;	// the object is yet to be created
        writeDataInp.rsComm = rsComm;

        // Set up easy handler
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, &irodsCurl::my_write_obj );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, &writeDataInp );
        curl_easy_setopt( curl, CURLOPT_URL, url );



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

        return res;
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

size_t curlStringRead(void *buffer, size_t size, size_t nmemb, void *stream) {
    dataStringURLsObject_t *urls = (dataStringURLsObject_t* )stream;

    size_t newbytes = size * nmemb;

    if (newbytes + urls->offset > MAX_SIZE) {
        newbytes = MAX_SIZE - urls->offset;
    }

    memcpy(urls->data_array + urls->offset, buffer, newbytes);
    urls->offset += newbytes;

    return newbytes;
}

// =-=-=-=-=-=-=-
// 1. Write a standard issue microservice
    int msiConvertFile(msParam_t* url, msParam_t* path, msParam_t* dest_obj, msParam_t* _out, ruleExecInfo_t* rei ) {
        dataObjInp_t destObjInp, *myDestObjInp;	/* for parsing input object */

	char *pathStr;
	pathStr = parseMspForStr (path);

	char *urlStr;
        urlStr = parseMspForStr (url);

	CURL *curl;
  	CURLcode res;
  	struct stat file_info;
  	double speed_upload, total_time;
  	FILE *fd;

	char *outStr;
        outStr = (char*)malloc(MAX_SIZE);

	struct curl_slist *headerlist = NULL;
	struct curl_httppost *post = NULL;
    	struct curl_httppost *last = NULL;
	static const char buf[] = "Expect:";

    	dataStringURLsObject_t dataStringURLsObject;
  	fd = fopen(pathStr, "rb"); /* open file to upload */ 


    	/* pad data structures with null chars */
    	memset(&dataStringURLsObject, 0, sizeof (dataStringURLsObject_t));


	
	if(!fd) {

    	return 1; /* can't continue */ 
  	}


  	/* to get the file size */ 
  	if(fstat(fileno(fd), &file_info) != 0) {

    		return 1; /* can't continue */ 
  	}
	

	curl_formadd(&post,
               &last,
               CURLFORM_COPYNAME, "send",
               CURLFORM_FILE, fd,
               CURLFORM_END);

	curl_formadd(&post,
               &last,
               CURLFORM_COPYNAME, "submit",
               CURLFORM_COPYCONTENTS, "true",
               CURLFORM_END);

  	curl = curl_easy_init();

	headerlist = curl_slist_append(headerlist, buf);

  	if(curl) {
    		/* upload to this place */
   	 	curl_easy_setopt(curl, CURLOPT_URL, urlStr);

    		/* tell it to "upload" to the URL */
    		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		/* set where to read from (on Windows you need to use READFUNCTION too) */
    		curl_easy_setopt(curl, CURLOPT_READDATA, fd);

    		/* and give the size of the upload (optional) */ 
    		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)file_info.st_size);

    		/* enable verbose for easier tracing */ 
    		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlStringRead);
	        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dataStringURLsObject);

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    		curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);



	    	res = curl_easy_perform(curl);
	    	/* Check for errors */ 
  	  	if(res != CURLE_OK) {
      			snprintf(outStr, MAX_NAME_LEN, "curl_easy_perform() failed");

	        	fillStrInMsParam (_out, outStr);
			return 1;
	    	}
    		else {
      			/* now extract transfer info */ 
      			curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed_upload);
      			curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);

      			snprintf(outStr,MAX_NAME_LEN, "Speed: bytes/sec %f during seconds %f \n", speed_upload, total_time);

   	 	}

		//char links[MAX_SIZE]; // = dataStringURLsObject . data;
	        //strncpy(links, dataStringURLsObject.data_array, MAX_SIZE);
       		//rodsLog(LOG_ERROR, "Links %s", links);
		
		//snprintf(outStr,MAX_SIZE, "%s", links);
		/*
		char *outStr;
		outStr = (char*)malloc(MAX_NAME_LEN);
		*/
		snprintf(outStr, MAX_SIZE, "path %s", dataStringURLsObject.data_array);
		
		fillStrInMsParam (_out, outStr);


    		/* always cleanup */ 
    		curl_easy_cleanup(curl);
  	}
	else{
		snprintf(outStr, MAX_NAME_LEN, "Curl didn't run");

        	fillStrInMsParam (_out, outStr);
		return 1;
	}

	/*
	// Sanity checks
        if ( !rei || !rei->rsComm ) {
            rodsLog( LOG_ERROR, "irods_curl_get: Input rei or rsComm is NULL." );
            return ( SYS_INTERNAL_NULL_INPUT_ERR );
        }

        // Get path of destination object
        rei->status = parseMspForDataObjInp( dest_obj, &destObjInp, &myDestObjInp, 0 );
        if ( rei->status < 0 ) {
            rodsLog( LOG_ERROR, "irods_curl_get: Input object error. status = %d", rei->status );
            return ( rei->status );
        }

        // Create irodsCurl instance
        irodsCurl myCurl( rei->rsComm );
	
        // Call irodsCurl::get
        rei->status = myCurl.get( parseMspForStr( url ), destObjInp.objPath );

        // Done
        return rei->status;
	*/

	return 0;
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
        msvc->add_operation( "msiConvertFile", "msiConvertFile" );

        // =-=-=-=-=-=-=-
        // 5. return the newly created microservice plugin
        return msvc;
    }



}	// extern "C"


