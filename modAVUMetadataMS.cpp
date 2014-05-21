/*
 * modAVUMetadataMS.c
 *
 *  Created on: July 5th, 2012
 *      Author: Alexandru Nedelcu
 *
 *  Updated by : Andrew Hochstetter, May 16th, 2014
 *
 *  Copyright 2014 Drexel University
 */
#include "msParam.hpp"
#include "reGlobalsExtern.hpp"
#include "irods_ms_plugin.hpp"
#include "apiHeaderAll.hpp"

#include <string>
#include <iostream>

extern int rsModAVUMetadata(rsComm_t *rsComm, modAVUMetadataInp_t *modAVUMetadataInp);

extern "C" {

int msiModAVUMetadata(msParam_t *inPath, msParam_t *inAttr, msParam_t *inVal, msParam_t *inUnit, msParam_t *outParam, ruleExecInfo_t* rei)
{
	char *outStr;
	char *pathStr;
	char *attrStr;
	char *valStr;
	char *unitStr;

        
	/*************************************  USUAL INIT PROCEDURE **********************************/

	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiHelloWorld")


	/********************************** RETRIEVE INPUT PARAMS **************************************/
	pathStr = parseMspForStr (inPath);
	attrStr = parseMspForStr (inAttr);
        valStr = parseMspForStr (inVal);
	unitStr = parseMspForStr (inUnit);


	/********************************** DO SOMETHING ***********************************************/
	outStr = (char*)malloc(MAX_NAME_LEN);
	snprintf(outStr, MAX_NAME_LEN, "Hello World!\nOur input string was: %s %s %s %s", pathStr, attrStr, valStr, unitStr);
	

        int returnCode;
        modAVUMetadataInp_t modAVUMetadataInp;

        memset(&modAVUMetadataInp, 0, sizeof (modAVUMetadataInp_t));
        modAVUMetadataInp.arg0 = "add";
        modAVUMetadataInp.arg1 = "-d"; /* -d for file */
        modAVUMetadataInp.arg2 = pathStr; /* Do I need to malloc something new and copy it in? */
        modAVUMetadataInp.arg3 = attrStr;
        modAVUMetadataInp.arg4 = valStr;
        modAVUMetadataInp.arg5 = unitStr;

        /* invoke rsModAVUMetadata() */
        returnCode = rsModAVUMetadata(rei->rsComm, &modAVUMetadataInp);

        if(returnCode == CATALOG_ALREADY_HAS_ITEM_BY_THAT_NAME) {
            return 0;
        }

          /********************************** WRITE STUFF TO OUT PARAMS **********************************/
        fillStrInMsParam (outParam, outStr);


        /**************************************** WE'RE DONE *******************************************/
        return 0;
    }

    irods::ms_table_entry* plugin_factory() {

        // =-=-=-=-=-=-=-
        // 3. Allocate a microservice plugin which takes the number of function
        //    params as a parameter to the constructor, not including rei. With
        //    N as the total number of arguments of my_microservice() we would have:
        irods::ms_table_entry* msvc = new irods::ms_table_entry(5);

        // =-=-=-=-=-=-=-
        // 4. Add the microservice function as an operation to the plugin
        //    the first param is the name / key of the operation, the second
        //    is the name of the function which will be the microservice
        msvc->add_operation("msiModAVUMetadata", "msiModAVUMetadata");

        // =-=-=-=-=-=-=-
        // 5. Return the newly created microservice plugin
        return msvc;
    }

};


