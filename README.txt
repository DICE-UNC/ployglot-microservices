Requirements 
	- irods 4.0 runtime and irods developers kit are installed on server
	- Makefile and .cpp files for microservices are within server director prior to being compiled

Tutorial for configuring new microservices can be found at https://github.com/irods/irods/blob/master/examples/microservices/microservice_tutorial.rst


To compile the microservices, use the commande "Make" followed by the following tag for each respective microservice...


irods_curl.cpp: Make curl_get


modAVUMetadataMS.cpp: Make modAVUMetadata

deleteAVUMetadata.cpp: Make deleteAVUMetadata

magicNumberMS.cpp: Make magicNumber



-------------------------
Running the microservices
-------------------------

Curl: irods_curl_get(*url, *source_object, *ext_object, *out );

Mod AVU: modAVUMetadataMS("Test_Path", "Test_Attribute", "Test_Value", "Test_unit", *out);

Mod AVU: deleteAVUMetadata("Test_Path", "Test_Attribute", "Test_Value", *out);

