/*
 * helloWorldMS.c
 *
 *  Created on: May 3, 2012
 *      Author: ids
 */

#include "apiHeaderAll.hpp"
#include "msParam.hpp"
#include "reGlobalsExtern.hpp"
#include "irods_ms_plugin.hpp"
#include "magicNumberMS.hpp"

extern int rsDataObjWrite (rsComm_t *rsComm, 
               openedDataObjInp_t *dataObjWriteInp,
               bytesBuf_t *dataObjWriteInpBBuf);
extern int rsDataObjRead (rsComm_t *rsComm, 
               openedDataObjInp_t *dataObjReadInp,
               bytesBuf_t *dataObjReadOutBBuf);  
extern int rsDataObjOpen (rsComm_t *rsComm, dataObjInp_t *dataObjInp);
extern int rsDataObjClose (rsComm_t *rsComm, openedDataObjInp_t *dataObjCloseInp);
extern int rsDataObjCreate (rsComm_t *rsComm, dataObjInp_t *dataObjInp);

extern int rsModAVUMetadata(rsComm_t *rsComm, modAVUMetadataInp_t *modAVUMetadataInp);

extern "C" {



    int msiMagicNumber(msParam_t *inParam, msParam_t *outParam, ruleExecInfo_t* rei) 
    {
       int returnCode;
       char *inStr;
       char *outStr;

       /*************************************  USUAL INIT PROCEDURE **********************************/

       /* For testing mode when used with irule --test */
       RE_TEST_MACRO("    Calling msiMagicNumber")

            /* Sanity checks */
       if (rei == NULL || rei->rsComm == NULL) {
          rodsLog(LOG_ERROR, "msiMagicNumber: input rei or rsComm is NULL.");
          return (SYS_INTERNAL_NULL_INPUT_ERR);
       }


       /********************************** RETRIEVE INPUT PARAMS **************************************/
       inStr = parseMspForStr(inParam);


       /********************************** DO SOMETHING ***********************************************/
       returnCode = __magicNumber(inStr, &outStr, rei->rsComm);

       returnCode = __writeAVUForFile(inStr, "magic_number", outStr, "HEX", rei->rsComm);


       /********************************** WRITE STUFF TO OUT PARAMS **********************************/
       fillStrInMsParam(outParam, outStr);

       /**************************************** WE'RE DONE *******************************************/
       return returnCode;
    }

    int __magicNumber(char *filePath, char **magicNumber, rsComm_t *rsComm) {

        int returnCode;
        dataObjInp_t *dataObjInp;
        openedDataObjInp_t *openedDataObjInp;
        bytesBuf_t *dataObjReadOutBBuf = NULL;

        dataObjInp = (dataObjInp_t*) malloc(sizeof (dataObjInp_t));
        memset(dataObjInp, 0, sizeof (dataObjInp_t));

        snprintf(dataObjInp->objPath, MAX_NAME_LEN, "%s", filePath);

        returnCode = rsDataObjOpen(rsComm, dataObjInp);
        if (returnCode < 0) {
           rodsLog(LOG_ERROR, "__magicNumber: PROBLEM OPENING DATA OBJECT. Status =  %d", returnCode);
           return returnCode;
        }

        openedDataObjInp = (openedDataObjInp_t*) malloc(sizeof (openedDataObjInp_t));
        memset(openedDataObjInp, 0, sizeof (openedDataObjInp_t));
        openedDataObjInp->l1descInx = returnCode;

        openedDataObjInp->len = MAGIC_NUMBER_LEN;
        dataObjReadOutBBuf = (bytesBuf_t *) malloc(sizeof (bytesBuf_t));
        memset(dataObjReadOutBBuf, 0, sizeof (bytesBuf_t));


        returnCode = rsDataObjRead(rsComm, openedDataObjInp, dataObjReadOutBBuf);
        if (returnCode < 0) {
            rodsLog(LOG_ERROR, "__magicNumber: PROBLEM READING FILE. Status =  %d", returnCode);
            return returnCode;
        }

        *magicNumber = (char*) malloc(MAGIC_NUMBER_HEX_LEN);

        unsigned char tmp[4];
        memset(tmp, 0, sizeof(tmp));
        memcpy(tmp, dataObjReadOutBBuf->buf, returnCode);
        snprintf(*magicNumber, MAGIC_NUMBER_HEX_LEN, "0x%02X%02X%02X%02X", tmp[0], tmp[1], tmp[2], tmp[3]);

        returnCode = rsDataObjClose(rsComm, openedDataObjInp);
        if (returnCode < 0) {
           rodsLog(LOG_ERROR, "__magicNumber: PROBLEM CLOSING FILE. Status =  %d", returnCode);
           return returnCode;
        }

        return 0;
    }

    int __writeAVUForFile(char* path, char* attribute, char* value, char* units, rsComm_t *rsComm) {
        int returnCode;
        modAVUMetadataInp_t modAVUMetadataInp;

        memset(&modAVUMetadataInp, 0, sizeof (modAVUMetadataInp_t));
        modAVUMetadataInp.arg0 = "add";
        modAVUMetadataInp.arg1 = "-d"; /* -d for file */
        modAVUMetadataInp.arg2 = path; /* Do I need to malloc something new and copy it in? */
        modAVUMetadataInp.arg3 = attribute;
        modAVUMetadataInp.arg4 = value;
        modAVUMetadataInp.arg5 = units;

        /* invoke rsModAVUMetadata() */
        returnCode = rsModAVUMetadata(rsComm, &modAVUMetadataInp);

        if(returnCode == CATALOG_ALREADY_HAS_ITEM_BY_THAT_NAME) {
            return 0;
        }

        return returnCode;
    }

    irods::ms_table_entry* plugin_factory() {
        // =-=-=-=-=-=-=-
        // 3. allocate a microservice plugin which takes the number of function
        // params as a parameter to the constructor
        irods::ms_table_entry* msvc = new irods::ms_table_entry( 2 );

        // =-=-=-=-=-=-=-
        // 4. add the microservice function as an operation to the plugin
        // the first param is the name / key of the operation, the second
        // is the name of the function which will be the microservice
        msvc->add_operation( "msiMagicNumber", "msiMagicNumber" );

        // =-=-=-=-=-=-=-
        // 5. return the newly created microservice plugin
        return msvc;
    }
};

