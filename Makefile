GCC = g++ 
INC=-I/usr/include/irods/

all: modAVU magicNumber

modAVU:
	${GCC} ${INC} -fPIC -shared -o libmodAVUMetadataMS.so modAVUMetadataMS.cpp -Wno-deprecated /usr/lib/irods/libirods_client.a

magicNumber:
        ${GCC} ${INC} -fPIC -shared -o libmsiMagicNumber.so magicNumberMS.cpp -Wno-deprecated /usr/lib/irods/$

hello:
        ${GCC} ${INC} -fPIC -shared -o libirods_hello.so irods_hello.cpp -Wno-deprecated /usr/lib/irods/libirods_client.a
        ${GCC} ${INC} -fPIC -shared -o libirods_hello2.so irods_hello.cpp -Wno-deprecated /usr/lib/irods/libirods_client.a



clean:
	@rm -f libmodAVUMetadataMS.so
