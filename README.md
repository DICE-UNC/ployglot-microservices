##Requirements 

* g++ is installed on machine (sudo apt-get install g++)
* irods 4.0 development tools is installed
* irods 4.0 runtime libraries are installed
* libcurl-dev is installed with openssl headers

Tutorial for configuring new microservices can be found at <https://github.com/irods/irods/blob/master/examples/microservices/microservice_tutorial.rst>

## Compiling

To compile the microservices, use the commande "make" followed by the following tag for each respective microservice...

irods\_curl.cpp:

    make curl_get

modAVUMetadataMS.cpp:

    make modAVUMetadata

deleteAVUMetadata.cpp:

    make deleteAVUMetadata

## Running the microservices

Curl:

    irods_curl_get(*url, *source_object, *ext_object, *out);

Mod AVU:

    modAVUMetadataMS("Test_Path", "Test_Attribute", "Test_Value", "Test_unit", *out);

Mod AVU:

    deleteAVUMetadata("Test_Path", "Test_Attribute", "Test_Value", "Test_unit", *out);

## Core.re configuration

Within the Core.re file the following microservice, use the following confirguaration for automatic implementation of polyclone conversion

    acPostProcForModifyAVUMetadata(*Option,*ItemType,*ItemName,*AName,*AValue,*AUnit) { 
    	on(*AName == "ConvertMe") {
    		irods_curl_get("http://polyglot.cci.drexel.edu/", *ItemName, *AValue, *out);
    		if(*out == ""){
    			deleteAVUMetadata(*ItemName, "ConvertMe", *AValue, *AUnit, *out3);
    			modAVUMetadata(*ItemName, "Conversion Error", *AValue, "dest", *out2);
    		}else{
    			modAVUMetadata(*out, "Derived from", *ItemName, "iRODS path", *out2);
    			deleteAVUMetadata(*ItemName, "ConvertMe", *AValue, *AUnit, *out3); 	
    		}
    	}
    }
    acPostProcForModifyAVUMetadata(*Option,*ItemType,*ItemName,*AName,*AValue,*AUnit) { }
    acPostProcForModifyAVUMetadata(*Option,*ItemType,*ItemName,*AName,*AValue) { }

The URL passsed to `irods_curl_get` <http://polyglot.cci.drexel.edu> must be an available instance of the "Polyclone" server. <https://bitbucket.org/drexel/polyclone>
