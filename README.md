##Requirements 

* g++ is installed on machine (sudo apt-get install g++)
* irods 4.0 development tools is installed
* irods 4.0 runtime libraries are installed
* libcurl-dev is installed with openssl headers

Tutorial for configuring new microservices can be found at <https://github.com/irods/irods/blob/master/examples/microservices/microservice_tutorial.rst>

## Compiling

To compile the microservices, use the commande "make" followed by the following tag for each respective microservice...

irods\_curl.cpp: make curl\_get

modAVUMetadataMS.cpp: make modAVUMetadata

deleteAVUMetadata.cpp: make deleteAVUMetadata


## Running the microservices

Curl: irods\_curl\_get(\*url, \*source\_object, \*ext\_object, \*out );

Mod AVU: modAVUMetadataMS("Test\_Path", "Test\_Attribute", "Test\_Value", "Test\_unit", \*out);

Mod AVU: deleteAVUMetadata("Test\_Path", "Test\_Attribute", "Test\_Value", "Test\_unit", \*out);


## Core.re configuration


Within the Core.re file the following microservice, use the following confirguaration for automatic implementation of polyclone conversion

    acPostProcForModifyAVUMetadata(*Option,*ItemType,*ItemName,*AName,*AValue,*AUnit) { 
    	on(*AName == "ConvertMe") {
    		irods_curl_get("http://144.118.173.125:5000/", *ItemName, *AValue, *out);
    		if(*out == ""){
    			deleteAVUMetadata(*ItemName, "ConvertMe", *AValue, *AUnit, *out3);
    			modAVUMetadata(*ItemName, "Conversion Error", *AValue, "dest", *out2);
    		}else{
    			modAVUMetadata(*out, "Derived from", *ItemName, "source", *out2);
    			deleteAVUMetadata(*ItemName, "ConvertMe", *AValue, *AUnit, *out3); 	
    		}
    	}
    }
    acPostProcForModifyAVUMetadata(*Option,*ItemType,*ItemName,*AName,*AValue,*AUnit) { }
    acPostProcForModifyAVUMetadata(*Option,*ItemType,*ItemName,*AName,*AValue) { }
