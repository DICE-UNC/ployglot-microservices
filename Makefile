GCC = g++ 
INC=-I/usr/include/irods/

all: modAVUMetadata

modAVUMetadata:
	${GCC} ${INC} -fPIC -shared -o libmodAVUMetadataMS.so modAVUMetadataMS.cpp -Wno-deprecated /usr/lib/irods/libirods_client.a


clean:
	@rm -f libmodAVUMetadataMS.so
