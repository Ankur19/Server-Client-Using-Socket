CC = g++
CFLAGS = -g -Wall
CPPVERSION = -std=c++17
LDFLAGS = -lssl -lcrypto -lz -ldl -static-libgcc "-lstdc++fs"

FileUtils.o : FileUtils/FileUtils.cpp FileUtils/FileUtils.h
			${CC} ${CPPVERSION} -c FileUtils/FileUtils.cpp ${LDFLAGS}

SerializationUtils.o : SerializationUtils/SerializationUtils.cpp SerializationUtils/SerializationUtils.h
			${CC} ${CPPVERSION} -c SerializationUtils/SerializationUtils.cpp ${LDFLAGS}

Server.o : Server.cpp
			${CC} ${CPPVERSION} -c Server.cpp ${LDFLAGS}

Client.o : Client.cpp
			${CC} ${CPPVERSION} -c Client.cpp ${LDFLAGS}

all : FileUtils.o SerializationUtils.o Server.o Client.o
			${CC} ${CPPVERSION} ${CFLAGS} FileUtils.o SerializationUtils.o Server.o -o server ${LDFLAGS}
			${CC} ${CPPVERSION} ${CFLAGS} FileUtils.o SerializationUtils.o Client.o -o client ${LDFLAGS}

run-server : server
			./server

run-client : client
			./client

clean :
			rm server client *.o