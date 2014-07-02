// =-=-=-=-=-=-=-
#include "msParam.hpp"
#include "reGlobalsExtern.hpp"
#include "irods_ms_plugin.hpp"
#include "apiHeaderAll.hpp"

// =-=-=-=-=-=-=-
// STL Includes
#include <iostream>

extern int rsModAVUMetadata(rsComm_t *rsComm, modAVUMetadataInp_t *modAVUMetadataInp);

extern "C" {

    // =-=-=-=-=-=-=-
    // 1. Write a standard issue microservice
    int irods_hello(msParam_t *inPath, msParam_t *inAttr, msParam_t *inVal,
                    msParam_t* _out, ruleExecInfo_t* _rei ) {
        std::string my_str = parseMspForStr(inAttr);

	int returnCode;
        char *outStr;
	char *pathStr;
	char *attrStr;
	char *valStr;

	/*************************************  USUAL INIT PROCEDURE **********************************/

	pathStr = parseMspForStr (inPath);
	attrStr = parseMspForStr (inAttr);
	valStr = parseMspForStr (inVal);
	outStr = (char*)malloc(MAX_NAME_LEN);
	snprintf(outStr, MAX_NAME_LEN, "Hello World!\nOur input string was: %s %s %s", pathStr, attrStr, valStr);

	fillStrInMsParam (_out, outStr);

 	modAVUMetadataInp_t modAVUMetadataInp;

        memset(&modAVUMetadataInp, 0, sizeof (modAVUMetadataInp_t));
        modAVUMetadataInp.arg0 = "rm";
        modAVUMetadataInp.arg1 = "-d"; /* -d for file */
        modAVUMetadataInp.arg2 = pathStr; /* Do I need to malloc something new and copy it in? */
        modAVUMetadataInp.arg3 = attrStr;
        modAVUMetadataInp.arg4 = valStr;
	modAVUMetadataInp.arg5 = "unit";
	returnCode = rsModAVUMetadata(_rei->rsComm, &modAVUMetadataInp);

	return 0;
    }

    // =-=-=-=-=-=-=-
    // 2.  Create the plugin factory function which will return a microservice
    //     table entry
    irods::ms_table_entry*  plugin_factory() {
        // =-=-=-=-=-=-=-
        // 3. allocate a microservice plugin which takes the number of function
        //    params as a parameter to the constructor
        irods::ms_table_entry* msvc = new irods::ms_table_entry( 4 );

        // =-=-=-=-=-=-=-
        // 4. add the microservice function as an operation to the plugin
        //    the first param is the name / key of the operation, the second
        //    is the name of the function which will be the microservice
        msvc->add_operation( "irods_hello", "irods_hello" );

        // =-=-=-=-=-=-=-
        // 5. return the newly created microservice plugin
        return msvc;
    }

}; // extern "C"



