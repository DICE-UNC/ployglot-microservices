#include "msParam.hpp"
#include "reGlobalsExtern.hpp"
#include "irods_ms_plugin.hpp"
#include "apiHeaderAll.hpp"

#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>

extern "C" {

	int msiGetExtension(msParam_t *fileName, msParam_t *outParam, ruleExecInfo_t* rei)
	{

   		char* ext;
    		char* p;
    		char* fullname = parseMspForStr(fileName);

		char* outStr;
		outStr = (char*)malloc(MAX_NAME_LEN);



    		ext = strrchr (fullname, '.');

		if (ext == NULL)
        		ext = "";

		snprintf(outStr, MAX_NAME_LEN, "%s", ext);

		fillStrInMsParam (outParam, outStr);


	}

    	irods::ms_table_entry* plugin_factory() {

        	// =-=-=-=-=-=-=-
        	// 3. Allocate a microservice plugin which takes the number of function
        	//    params as a parameter to the constructor, not including rei. With
        	//    N as the total number of arguments of my_microservice() we would have:
        	irods::ms_table_entry* msvc = new irods::ms_table_entry(2);

        	// =-=-=-=-=-=-=-
        	// 4. Add the microservice function as an operation to the plugin
        	//    the first param is the name / key of the operation, the second
        	//    is the name of the function which will be the microservice
        	msvc->add_operation("msiGetExtension", "msiGetExtension");

        	// =-=-=-=-=-=-=-
        	// 5. Return the newly created microservice plugin
        	return msvc;
    	}

};
