GCC = g++ 
INC=-I/usr/include/irods/

all: ext size

ext:
	${GCC} ${INC} -fPIC -shared -o libmsiGetExtension.so msiGetExtension.cpp -Wno-deprecated /usr/lib/irods/libirods_client.a

size:
	${GCC} ${INC} -DRODS_SERVER -fPIC -shared -g -Wno-deprecated -o libirods_size2.so irods_size2.cpp  /usr/lib/irods/libirods_client.a
	
clean:
	@rm -f lib*.so

