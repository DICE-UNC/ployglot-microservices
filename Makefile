GCC = g++ 
INC=-I/usr/include/irods/

all: msvc_test hello curl_get

msvc_test:
	${GCC} ${INC} -fPIC -shared -Wno-deprecated -o libirods_msvc_test.so irods_msvc_test.cpp /usr/lib/libirods.a
	
hello:
	${GCC} ${INC} -fPIC -shared -o libirods_hello.so irods_hello.cpp -Wno-deprecated /usr/lib/irods/libirods_client.a

ext:
	${GCC} ${INC} -fPIC -shared -o libmsiGetExtension.so msiGetExtension.cpp -Wno-deprecated /usr/lib/irods/libirods_client.a


modAVU: 
	${GCC} ${INC} -fPIC -shared -o libmodAVUMetadata.so modAVUMetadata.cpp -Wno-deprecated /usr/lib/irods/libirods_client.a 

modAVUMetadata:
	${GCC} ${INC} -fPIC -shared -o libmodAVUMetadataMS.so modAVUMetadataMS.cpp -Wno-deprecated /usr/lib/irods/libirods_client.a


magicNumber:
	${GCC} ${INC} -fPIC -shared -o libmsiMagicNumber.so magicNumberMS.cpp -Wno-deprecated /usr/lib/irods/libirods_client.a


msiConvertFile:
	${GCC} ${INC} -fPIC -shared -g -Wno-deprecated -o libmsiConvertFile.so msiConvertFile.cpp `curl-config --libs` /usr/lib/irods/libirods_client.a

curl_get:
	${GCC} ${INC} -DRODS_SERVER -fPIC -shared -g -Wno-deprecated -o libirods_curl_get.so irods_curl.cpp `curl-config --libs` /usr/lib/irods/libirods_client.a

size:
	${GCC} ${INC} -DRODS_SERVER -fPIC -shared -g -Wno-deprecated -o libirods_size2.so irods_size2.cpp  /usr/lib/irods/libirods_client.a
	
clean:
	@rm -f libirods_msvc_test.so

