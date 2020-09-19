CC = g++
CFLAGS = -g -Wall
CPPVERSION = -std=c++17
LDFLAGS = "-lstdc++fs"

FileUtils.o : FileUtils/FileUtils.cpp FileUtils/FileUtils.h
			${CC} ${CPPVERSION} -c FileUtils/FileUtils.cpp ${LDFLAGS}

SerializationUtils.o : SerializationUtils/SerializationUtils.cpp SerializationUtils/SerializationUtils.h
			${CC} ${CPPVERSION} -c SerializationUtils/SerializationUtils.cpp ${LDFLAGS}

Server.o : Server.cpp
			${CC} ${CPPVERSION} -c Server.cpp ${LDFLAGS}

all : FileUtils.o SerializationUtils.o Server.o
			${CC} ${CPPVERSION} ${CFLAGS} FileUtils.o SerializationUtils.o Server.o -o server ${LDFLAGS}

run : server
			./server

clean :
			rm server *.o