GCC = g++ 
INC=-I/usr/include/irods/

all: deleteAVUMetadata modAVUMetadata curl_get

deleteAVUMetadata: 
	${GCC} ${INC} -fPIC -shared -o libdeleteAVUMetadata.so deleteAVUMetadata.cpp -Wno-deprecated /usr/lib/irods/libirods_client.a 

modAVUMetadata:
	${GCC} ${INC} -fPIC -shared -o libmodAVUMetadataMS.so modAVUMetadataMS.cpp -Wno-deprecated /usr/lib/irods/libirods_client.a

curl_get:
	${GCC} ${INC} -DRODS_SERVER -fPIC -shared -g -Wno-deprecated -o libirods_curl_get.so irods_curl.cpp `curl-config --libs` /usr/lib/irods/libirods_client.a

clean:
	@rm -f libirods_msvc_test.so

