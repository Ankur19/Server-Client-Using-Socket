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
#include <pthread.h>
#include "TransferUtils/TransferUtils.h"

using namespace std;

#define PACKET_SIZE 1024
#define SIZE_PACKET_SIZE 8
#define LOCALHOST "127.0.0.1"
#define PORT_NUMBER 50015
#define MAX_CLIENTS 8

void* connectToClient(void* td);

struct ThreadData{
	int clientDescriptor;
	string serializedFile;
	FileUtils::FileList* f;
	ThreadData(): clientDescriptor(0), serializedFile(""), f(nullptr) {}
	ThreadData(int cd, string& s, FileUtils::FileList* fl): clientDescriptor(cd), serializedFile(s), f(fl) {}
};

int main()
{
    // Instantiate char Array to be sent and received
	char dataSending[PACKET_SIZE];
	char dataReceiving[PACKET_SIZE];
	pthread_t threads[MAX_CLIENTS];
	ThreadData* td = nullptr;
	string tempString = "";
	FileUtils::FileList* f = nullptr;
	string serializedFile = "";
	char* temp;
	int start;
	int size;

    // Instantiate the Listener and Connection variables
	int clintListn = 0, clintConnt = 0, fd = 0, n = 0, currentThread = 0;

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

    
	while(1)
	{
        cout << "Server started..." << endl;
        cout << "Listening on: " << LOCALHOST << ":" << PORT_NUMBER << endl; 
		cout << "PWD: " << FileUtils::getPwd() << endl;
		if(!f){
			f = FileUtils::getFilesInDir(FileUtils::getPwd() );
			serializedFile = SerializationUtils::serializeFileList(*f);
			td = new ThreadData(0, serializedFile, f);
		}

		clintConnt = accept(clintListn, (struct sockaddr*)NULL, NULL);
		td->clientDescriptor = clintConnt;
		n = pthread_create(&threads[currentThread], NULL, connectToClient, (void*)td);

		if (n) {
			cout << "Error:unable to create thread," << n << endl;
			exit(-1);
		}
		currentThread++;
     }
 
     return 0;
}

void* connectToClient(void* td){

	ThreadData* threadData = (ThreadData*)td;
	string tempString = "";
	int n;

	cout << "Client Number: " << threadData->clientDescriptor << endl;

	TransferUtils::sendFile(threadData->serializedFile, threadData->clientDescriptor);

	cout << "File Info sent" << endl;

	
	cout << "Waiting for response from client" << endl;
	
	n = TransferUtils::receiveSize(threadData->clientDescriptor);
	
	cout << "Received file number: " << n << endl;
	if(n <=0 || n > threadData->f->numFiles){
		cout << "Invalid request from client." << endl;
		close(threadData->clientDescriptor);
		return nullptr;
	}

	TransferUtils::sendCustomFile(threadData->f->files[n-1], threadData->clientDescriptor);
	cout << "File sent to client" << endl;
	close(threadData->clientDescriptor);
	pthread_exit(NULL);
	return nullptr;
}