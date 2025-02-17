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

// Data structure to store data for multiple processes (As we use multiple processes instead of threads here)
struct ThreadData{
	int clientDescriptor;
	string serializedFile;
	vector<FileUtils::FileInfo> sentFiles;
	FileUtils::FileList* f;
	ThreadData(): clientDescriptor(0), serializedFile(""), sentFiles(vector<FileUtils::FileInfo>{}), f(nullptr) {}
	ThreadData(int cd, string& s, FileUtils::FileList* fl): clientDescriptor(cd), serializedFile(s), sentFiles(vector<FileUtils::FileInfo>{}), f(fl) {}
};

void* connectToClient(ThreadData* td, int clientNumber);

// Main
int main(int argc, const char* argv[])
{
    // Instantiate thread list and thread data list
	pthread_t threads[MAX_CLIENTS];
	struct sockaddr_in ipOfServer;
	struct sockaddr_storage serverStorage;
	socklen_t addrSize;
	FileUtils::FileList* f = nullptr;
	string serializedFile = "";
	vector<pid_t> pids(MAX_CLIENTS, 0);
	char* temp;
	int numClients = 0;
	int start = 0, size = 0, port = 0;
	int numThreads = 0;
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

	// Print the working directory and listening port
	cout << "Server started..." << endl;
	cout << "Listening on: " << LOCALHOST << ":" << port << endl; 
	cout << "PWD: " << FileUtils::getPwd() << endl;
	while(1)
	{
		// If the directory data structure is empty populate it as we send it to client each time one connects
		if(!f){
			f = FileUtils::getFilesInDir(FileUtils::getPwd());
			serializedFile = SerializationUtils::serializeFileList(*f);
		}

		// Accept() method accepts connection requests from clients
		addrSize = sizeof(serverStorage);
		numClients++;
		clintConnt = accept(clintListn, (struct sockaddr*)&serverStorage, &addrSize);
		
		// Have a check on the number of processes possible in the server
		if(numThreads<=MAX_CLIENTS){
			// Fork if limit is not reached
			n = fork();
			if(n==0){
				numThreads++;
				ThreadData* td = new ThreadData();
				td->serializedFile = serializedFile;
				td->f = f;
				td->clientDescriptor = clintConnt;
				connectToClient(td, numClients); // In child process call ConnectToClient()
				numThreads--;
			}
			else{
				pids[currentThread] = n;
			}
		}
		else{
			// Wait for all pid's when connection limit is reached and finally exit.
			cout << "Connection limit exceeded." << endl;
			for(pid_t i: pids)
				waitpid(i, NULL, 0);
		}

		currentThread++;
     }
 
     return 0;
}

void* connectToClient(ThreadData* td, int clientNumber){
	// Thread Data - Data structure passed from main
	ThreadData threadData = *td;
	
	// Instantiate variables
	string tempString = "";
	int n = 0, numFiles = 0, sharingType = 0;
	FileUtils::FileInfo f;
	vector<int> files;
	clock_t beginTime;

	// Print client number
	cout << "Client Number: " << threadData.clientDescriptor << endl;

	// Send the file list
	TransferUtils::sendFile(threadData.serializedFile, threadData.clientDescriptor);

	cout << "File Info sent" << endl;
	cout << "Waiting for response from client" << endl;
	
	// After sending the files list wait for the number of files and the procedure to send them
	numFiles = TransferUtils::receiveSize(threadData.clientDescriptor); // Number of files to send to client
	sharingType = TransferUtils::receiveSize(threadData.clientDescriptor); // Whether to send them sequentially or parallely
	
	// If sequential 
	if(sharingType==1){
		beginTime = clock();
		while(numFiles > 0){
			n = TransferUtils::receiveSize(threadData.clientDescriptor); // Receive the file Number first
			cout << "Received file number: " << n << endl;
			if(n <=0 || n > threadData.f->numFiles){
				cout << "Invalid request from client." << endl;
				close(threadData.clientDescriptor);
				delete td;
				return nullptr;
			}
			// Send the file to client
			f = TransferUtils::sendCustomFile(threadData.f->files[n-1], threadData.clientDescriptor, clientNumber);
			// log the files sent in the sentFiles list
			f.fileMd5 = threadData.f->md5[n-1];
			threadData.sentFiles.push_back(f);
			
			cout << "File sent to client" << endl;
			numFiles--;
			// Once all the files are sent we calculate the time taken and log it in time.txt
			if(numFiles==0){
				numFiles = TransferUtils::receiveSize(threadData.clientDescriptor);
				TransferUtils::printToFile(to_string((float)(clock()-beginTime)/CLOCKS_PER_SEC), "time.txt");
				cout << "Sent files in: " << (float)(clock()-beginTime)/CLOCKS_PER_SEC << "  seconds" << endl;
				beginTime = clock();
			}
		}
	}
	else{
		while(numFiles> 0){
			// If the user selection was to send files parallely, send then using multiple threads
			vector<FileUtils::FileInfo> sentFiles = TransferUtils::sendCustomFilesMultithreaded(numFiles, threadData.f, threadData.clientDescriptor, clientNumber);
		}
	}
	// Finallly close the client connection and exit the function.
	close(threadData.clientDescriptor);
	delete td;
	return nullptr;
}