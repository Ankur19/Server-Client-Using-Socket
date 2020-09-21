#include <stdio.h> // Standard input and output library
#include <stdlib.h> // This includes functions regarding memory allocation
#include <string.h> // Contains string functions
#include <errno.h> // It defines macros for reporting and retrieving error conditions through error codes
#include <time.h> // Contains various functions for manipulating date and time
#include <unistd.h> // Contains various constants
#include <sys/types.h> // Contains a number of basic derived types that should be used whenever appropriate
#include <arpa/inet.h> // Defines in_addr structure
#include <sys/socket.h> // For socket creation
#include <netinet/in.h> // Contains constants and structures needed for internet domain addresses
#include <bits/stdc++.h> // Standard C++ functionality
#include "SerializationUtils/SerializationUtils.h"

using namespace std;

#define PACKET_SIZE 1024
#define LOCALHOST "127.0.0.1"
#define PORT_NUMBER 50015
#define MAX_CLIENTS 8

int main()
{
    // Instantiate char Array to be sent and received
	char dataSending[PACKET_SIZE];
	char dataReceiving[PACKET_SIZE];
	string tempString = "";

    // Instantiate the Listener and Connection variables
	int clintListn = 0, clintConnt = 0, fd = 0, n = 0;

    // Instantiate the struct describing Internet Socket Address
	struct sockaddr_in ipOfServer;

    // Create the socket using AF_INET (IPv4 address) && SOCK_STREAM (TCP)
	clintListn = socket(AF_INET, SOCK_STREAM, 0); // creating socket

    // Cleaning the variables instantiated
	memset(&ipOfServer, '0', sizeof(ipOfServer));
	memset(dataSending, '0', sizeof(dataSending));

    // Filling up the Socket details
	ipOfServer.sin_family = AF_INET; // Address type= IPv4
	ipOfServer.sin_addr.s_addr = htonl(INADDR_ANY); // Host: Any
	ipOfServer.sin_port = htons(PORT_NUMBER); // Port Number: 5555

    // Bind socket with the ipOfServer
	bind(clintListn, (struct sockaddr*)&ipOfServer , sizeof(ipOfServer));

    // Finally listen to the port provided
	listen(clintListn , MAX_CLIENTS);

    FileList* f = nullptr;
	string serializedFile = "";
	char* temp;
	int start;
	int size;
	while(1)
	{
        cout << "Server started..." << endl;
        cout << "Listening on: " << LOCALHOST << ":" << PORT_NUMBER << endl; 
		cout << "PWD: " << FileUtils::getPwd() << endl;
		if(!f){
			f = FileUtils::getFilesInDir(FileUtils::getPwd() );
			serializedFile = SerializationUtils::serializeFileList(*f);
		}

		clintConnt = accept(clintListn, (struct sockaddr*)NULL, NULL);
		cout << "Client Number: " << clintConnt << endl;

		size = serializedFile.size();
		temp = (char* )malloc(size*sizeof(char));
		memcpy(temp, serializedFile.c_str(), size);
		while(size > 0){
			memset(dataSending, '\0', PACKET_SIZE);
			memcpy(dataSending, temp, PACKET_SIZE);
			write(clintConnt, dataSending, strlen(dataSending));
			temp+=PACKET_SIZE;
			size-=PACKET_SIZE;
		}
		tempString = "";
		while(1){
			cout << "Waiting for response from client" << endl;
			memset(dataReceiving, '\0', PACKET_SIZE);
			recv(clintConnt, dataReceiving, PACKET_SIZE, 0);
			tempString+=string(dataReceiving);
			SerializationUtils::rtrim(tempString);
			n = stoi(tempString);
			if(n>=1 && n <= f->numFiles)
				break;
			else{
				cout << "Invalid request from client." << endl;
				break;
			}
		}
		fd = open(f->files[n-1].c_str(), O_RDONLY);
		while((n = read(fd, dataReceiving, sizeof(dataReceiving))) > 0){
			write(clintConnt, dataReceiving, sizeof(dataReceiving));
		}
		close(fd);

        close(clintConnt);
     }
 
     return 0;
}