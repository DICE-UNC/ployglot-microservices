// =-=-=-=-=-=-=-
#include "msParam.hpp"
#include "reGlobalsExtern.hpp"
#include "irods_ms_plugin.hpp"
#include "apiHeaderAll.hpp"

// =-=-=-=-=-=-=-
// STL Includes
#include <iostream>


extern "C" {

    // =-=-=-=-=-=-=-
    // 1. Write a standard issue microservice



int irods_size2( msParam_t *inpParam1, msParam_t *outParam, ruleExecInfo_t *rei ) {
    rsComm_t *rsComm;
    dataObjInp_t dataObjInp, *myDataObjInp;
    rodsObjStat_t *rodsObjStatOut = NULL;

    RE_TEST_MACRO( " Calling msiObjStat" )

    if ( rei == NULL || rei->rsComm == NULL ) {
        rodsLog( LOG_ERROR,
                 "msiObjStat: input rei or rsComm is NULL" );
        return ( SYS_INTERNAL_NULL_INPUT_ERR );
    }

    rsComm = rei->rsComm;

    /* parse inpParam1 */
    rei->status = parseMspForDataObjInp( inpParam1, &dataObjInp,
                                         &myDataObjInp, 0 );

    if ( rei->status < 0 ) {
        rodsLogAndErrorMsg( LOG_ERROR, &rsComm->rError, rei->status,
                            "msiObjStat: input inpParam1 error. status = %d", rei->status );
        return ( rei->status );
    }

    rei->status = rsObjStat( rsComm, myDataObjInp, &rodsObjStatOut );


    int intSize = rodsObjStatOut->objSize;

    char size[20];

    snprintf(size,20, "%d", intSize);
    //fillStrInMsParam(_out, size);


    if ( rei->status >= 0 ) {
        fillMsParam( outParam, NULL, RodsObjStat_MS_T, rodsObjStatOut, NULL );
    }
    else {
        rodsLogAndErrorMsg( LOG_ERROR, &rsComm->rError, rei->status,
                            "msiObjStat: rsObjStat failed for %s, status = %d",
                            myDataObjInp->objPath,
                            rei->status );
    }

    fillStrInMsParam(outParam, size);

    return ( rei->status );
}
    // =-=-=-=-=-=-=-
    // 2.  Create the plugin factory function which will return a microservice
    //     table entry
    irods::ms_table_entry*  plugin_factory() {
        // =-=-=-=-=-=-=-
        // 3. allocate a microservice plugin which takes the number of function
        //    params as a parameter to the constructor
        irods::ms_table_entry* msvc = new irods::ms_table_entry( 2 );

        // =-=-=-=-=-=-=-
        // 4. add the microservice function as an operation to the plugin
        //    the first param is the name / key of the operation, the second
        //    is the name of the function which will be the microservice
        msvc->add_operation( "irods_size2", "irods_size2" );

        // =-=-=-=-=-=-=-
        // 5. return the newly created microservice plugin
        return msvc;
    }


}; // extern "C"
