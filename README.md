##Requirements 

* irods 4.0 runtime and irods developers kit are installed on server
* g++ is installed on machine (sudo apt-get install g++)
* irods development tools is installed
* irods runtime libraries are installed
* libcurl-dev is installed with openssl headers

Tutorial for configuring new microservices can be found at https://github.com/irods/irods/blob/master/examples/microservices/microservice_tutorial.rst

## Compiling

To compile the microservices, use the commande "make" followed by the following tag for each respective microservice...

irods_curl.cpp: make curl_get

modAVUMetadataMS.cpp: make modAVUMetadata

deleteAVUMetadata.cpp: make deleteAVUMetadata


## Running the microservices

Curl: irods_curl_get(*url, *source_object, *ext_object, *out );

Mod AVU: modAVUMetadataMS("Test_Path", "Test_Attribute", "Test_Value", "Test_unit", *out);

Mod AVU: deleteAVUMetadata("Test_Path", "Test_Attribute", "Test_Value", *out);

