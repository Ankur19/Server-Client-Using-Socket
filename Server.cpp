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
#include <sys/wait.h>
#include <vector>
#include "TransferUtils/TransferUtils.h"

using namespace std;

#define LOCALHOST "127.0.0.1"
#define MAX_CLIENTS 8

void* connectToClient(void* td);

struct ThreadData{
	int clientDescriptor;
	string serializedFile;
	vector<FileUtils::FileInfo> sentFiles;
	FileUtils::FileList* f;
	ThreadData(): clientDescriptor(0), serializedFile(""), sentFiles(vector<FileUtils::FileInfo>{}), f(nullptr) {}
	ThreadData(int cd, string& s, FileUtils::FileList* fl): clientDescriptor(cd), serializedFile(s), sentFiles(vector<FileUtils::FileInfo>{}), f(fl) {}
};

int main(int argc, const char* argv[])
{
    // Instantiate thread list and thread data list
	pthread_t threads[MAX_CLIENTS];
	struct sockaddr_in ipOfServer;
	struct sockaddr_storage serverStorage;
	socklen_t addrSize;
	vector<ThreadData*> threadDataList(MAX_CLIENTS, new ThreadData());
	FileUtils::FileList* f = nullptr;
	string serializedFile = "";
	vector<pid_t> pids(MAX_CLIENTS, 0);
	char* temp;
	int start = 0, size = 0, port = 0;
	int clintListn = 0, clintConnt = 0, n = 0, currentThread = 0; // Instantiate the Listener and Connection variables
	
	// Get port from main arguments
	port = stoi(argv[1]);

    // Create the socket using AF_INET (IPv4 address) && SOCK_STREAM (TCP)
	clintListn = socket(AF_INET, SOCK_STREAM, 0); // creating socket

    // Cleaning the variables instantiated
	memset(&ipOfServer, '0', sizeof(ipOfServer));

    // Filling up the Socket details
	ipOfServer.sin_family = AF_INET; // Address type= IPv4
	ipOfServer.sin_addr.s_addr = htonl(INADDR_ANY); // Host: Any
	ipOfServer.sin_port = htons(port); // Port Number: 50015

    // Bind socket with the ipOfServer
	bind(clintListn, (struct sockaddr*)&ipOfServer , sizeof(ipOfServer));

    // Finally listen to the port provided
	listen(clintListn , 2 * MAX_CLIENTS);

	cout << "Server started..." << endl;
	cout << "Listening on: " << LOCALHOST << ":" << port << endl; 
	cout << "PWD: " << FileUtils::getPwd() << endl;
	while(1)
	{
		if(!f){
			f = FileUtils::getFilesInDir(FileUtils::getPwd());
			serializedFile = SerializationUtils::serializeFileList(*f);
		}
		addrSize = sizeof(serverStorage);
		clintConnt = accept(clintListn, (struct sockaddr*)&serverStorage, &addrSize);
		
		threadDataList[currentThread]->clientDescriptor = clintConnt;
		threadDataList[currentThread]->f = f;
		threadDataList[currentThread]->serializedFile = serializedFile;
		/*n = pthread_create(&threads[currentThread], NULL, connectToClient, (void*)threadDataList[currentThread]);

		if (n) {
			cout << "Error:unable to create thread," << n << endl;
			exit(-1);
		};*/
		if(currentThread<=MAX_CLIENTS){
			n = fork();
			if(n==0){
				connectToClient(threadDataList[currentThread]);
			}
			else{
				pids[currentThread] = n;
			}
		}
		else{
			cout << "Connection limit exceeded." << endl;
			for(pid_t i: pids)
				waitpid(i, NULL, 0);
		}

		currentThread++;
     }
 
     return 0;
}

void* connectToClient(void* td){

	ThreadData* threadData = (ThreadData*)td;
	string tempString = "";
	int n = 0, numFiles = 0, sharingType = 0;
	FileUtils::FileInfo f;
	vector<int> files;
	clock_t beginTime;

	cout << "Client Number: " << threadData->clientDescriptor << endl;

	TransferUtils::sendFile(threadData->serializedFile, threadData->clientDescriptor);

	cout << "File Info sent" << endl;

	
	cout << "Waiting for response from client" << endl;
	
	numFiles = TransferUtils::receiveSize(threadData->clientDescriptor);
	sharingType = TransferUtils::receiveSize(threadData->clientDescriptor);
	if(sharingType==1){
		beginTime = clock();
		while(numFiles > 0){
			n = TransferUtils::receiveSize(threadData->clientDescriptor);
	
			cout << "Received file number: " << n << endl;
			if(n <=0 || n > threadData->f->numFiles){
				cout << "Invalid request from client." << endl;
				close(threadData->clientDescriptor);
				return nullptr;
			}

			f = TransferUtils::sendCustomFile(threadData->f->files[n-1], threadData->clientDescriptor);
			f.fileMd5 = threadData->f->md5[n-1];
			threadData->sentFiles.push_back(f);
			
			cout << "File sent to client" << endl;
			numFiles--;
			if(numFiles==0){
				numFiles = TransferUtils::receiveSize(threadData->clientDescriptor);
				TransferUtils::printToFile(to_string((float)(clock()-beginTime)/CLOCKS_PER_SEC));
				cout << "Sent files in: " << (float)(clock()-beginTime)/CLOCKS_PER_SEC << " milli seconds" << endl;
				beginTime = clock();
			}
		}
	}
	else{
		while(numFiles> 0){
			vector<FileUtils::FileInfo> sentFiles = TransferUtils::sendCustomFilesMultithreaded(numFiles, threadData->f, threadData->clientDescriptor);
		}
	}
	
	close(threadData->clientDescriptor);

	pthread_exit(NULL);
	return nullptr;
}