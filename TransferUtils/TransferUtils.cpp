#include "TransferUtils.h"

void TransferUtils::receiveFile(string& output, int fileDescriptor){
    char dataReceiving[PACKET_SIZE];
    int size = 0, finalSize = 0, n = 0;
    output = "";

    size = receiveSize(fileDescriptor);
    finalSize = size;
    while(size>0)
    {
        memset(dataReceiving, '0', PACKET_SIZE);
        n = recv(fileDescriptor, dataReceiving, PACKET_SIZE, 0);
        size-=n;
        output+= string(dataReceiving);
    }
    SerializationUtils::rtrim(output);
}

void TransferUtils::sendSize(int size, int fileDescriptor){
    char sizePacket[SIZE_PACKET_SIZE];
    memcpy(sizePacket, to_string(size).c_str(), SIZE_PACKET_SIZE);
	send(fileDescriptor, sizePacket, SIZE_PACKET_SIZE, 0);
    return;
}

int TransferUtils::receiveSize(int fileDescriptor){
    char sizePacket[SIZE_PACKET_SIZE];
    recv(fileDescriptor, sizePacket, SIZE_PACKET_SIZE, 0);
    return atoi(sizePacket);
}

void TransferUtils::sendFile(string& serializedFile, int clientDescriptor){
	int size = 0, n = 0;
    char dataSending[PACKET_SIZE];
	
	size = serializedFile.size();
	char* temp = (char* )malloc(size*sizeof(char));
	char* pointerToFree = temp;

    sendSize(size, clientDescriptor);
	memcpy(temp, serializedFile.c_str(), size);

	while(size > 0){
		memset(dataSending, ' ', PACKET_SIZE);
        memcpy(dataSending, temp, PACKET_SIZE);
        
        send(clientDescriptor, dataSending, PACKET_SIZE, 0);
        temp+=PACKET_SIZE;
        size-=PACKET_SIZE;
	}

	free(pointerToFree);
}

void TransferUtils::sendCustomFile(string fileLocation, int socketDescriptor){
    int size;
    char dataSending[PACKET_SIZE];

    int fd = open(fileLocation.c_str(), O_RDONLY);
    struct stat fileStat = FileUtils::getFileStat(fd);

    size = fileStat.st_size;

    sendSize(size, socketDescriptor);

    while(size > 0){
        memset(dataSending, ' ', PACKET_SIZE);
        read(fd, dataSending, PACKET_SIZE);
        send(socketDescriptor, dataSending, PACKET_SIZE, 0);
        size-=PACKET_SIZE;
    }
    close(fd);
}