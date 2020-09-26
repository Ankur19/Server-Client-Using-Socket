CC = g++
CFLAGS = -g -Wall
CPPVERSION = -std=c++17
LDFLAGS = -lpthread -lssl -lcrypto -lz -ldl -static-libgcc "-lstdc++fs"

FileUtils.o : FileUtils/FileUtils.cpp FileUtils/FileUtils.h
			${CC} ${CPPVERSION} -c FileUtils/FileUtils.cpp ${LDFLAGS}

SerializationUtils.o : SerializationUtils/SerializationUtils.cpp SerializationUtils/SerializationUtils.h
			${CC} ${CPPVERSION} -c SerializationUtils/SerializationUtils.cpp ${LDFLAGS}

TransferUtils.o : TransferUtils/TransferUtils.cpp TransferUtils/TransferUtils.h
			${CC} ${CPPVERSION} -c TransferUtils/TransferUtils.cpp ${LDFLAGS}

Server.o : Server.cpp
			${CC} ${CPPVERSION} -c Server.cpp ${LDFLAGS}

Client.o : Client.cpp
			${CC} ${CPPVERSION} -c Client.cpp ${LDFLAGS}

all : FileUtils.o SerializationUtils.o TransferUtils.o Server.o Client.o
			${CC} ${CPPVERSION} ${CFLAGS} FileUtils.o SerializationUtils.o TransferUtils.o Server.o -o server ${LDFLAGS}
			${CC} ${CPPVERSION} ${CFLAGS} FileUtils.o SerializationUtils.o TransferUtils.o Client.o -o client ${LDFLAGS}

run-server : server
			./server $(PORT)

run-client : client
			./client $(PORT) ${NUM_CLIENTS}

clean :
			rm server client *.o