GCC = g++ 
INC=-I/usr/include/irods/

all: modAVU magicNumber

modAVU:
	${GCC} ${INC} -fPIC -shared -o libmodAVUMetadataMS.so modAVUMetadataMS.cpp -Wno-deprecated /usr/lib/irods/libirods_client.a

magicNumber:
        ${GCC} ${INC} -fPIC -shared -o libmsiMagicNumber.so magicNumberMS.cpp -Wno-deprecated /usr/lib/irods/$

clean:
	@rm -f libmodAVUMetadataMS.so
