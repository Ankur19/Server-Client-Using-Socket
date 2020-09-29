#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include "TransferUtils/TransferUtils.h"

using namespace std;

void* connectToServer(void* tD);


// Data structure to store thread data
struct ThreadData{
	int clientDescriptor;
    int port;
	string serializedFile;
	vector<FileUtils::FileInfo> sentFiles;
	FileUtils::FileList* f;
    int automated;
    int threadNum;
	ThreadData(): clientDescriptor(0), serializedFile(""), sentFiles(vector<FileUtils::FileInfo>{}), f(nullptr), threadNum(0), port(0), automated(0) {}
};

int main(int argc, const char* argv[])
{
    // Instantiate some variables
    int clientSocket = 0,n = 0;
    
    // Get user input
    int port = stoi(argv[1]);
    int folderIndex = stoi(argv[2]);
    int automated = stoi(argv[3]);

    // Instantiate a thread list & a thread data list
    pthread_t threads[1];
    ThreadData* td = new ThreadData();
    vector<ThreadData*> threadDataList{td};

    // Loop over as many threads as we want. Hardcoding it to 1 because now we consider one client as a single process.
    // Open a new terminal and run-client to create a new client
    for(int i = 0; i<1;i++){
        threadDataList[i]->port = port;
        threadDataList[i]->threadNum = folderIndex;
        threadDataList[i]->automated = automated;

        // Create thread and call connectToServer Method
        n = pthread_create(&threads[i], NULL, connectToServer, (void*)threadDataList[i]);
        if (n) {
			cout << "Error:unable to create thread," << n << endl;
			exit(-1);
		};
    }

    // Wait for thread to finish execution
    for(pthread_t& pt: threads){
        pthread_join(pt, NULL);
    }

    // Delete the data structure
    delete threadDataList[0];
    exit(0);
}

void* connectToServer(void* tD){
    // Method Param
    ThreadData* threadData = (ThreadData*)tD;
    
    // Instantiate variables
    int clientSocket, threadNum, n = 0, port = 0;
    string tempString, newFileLocation;
    vector<int> files{};
    bool isValid;
    FileUtils::FileList f;
    struct sockaddr_in ipOfServer;

    // Get the threadNumber (Or in this case this is the FolderIndex)
    threadNum = threadData->threadNum;
    // Get the port
    port = threadData->port;
    // Create a directory names ClientFolder_FolderIndex
    n = mkdir((FileUtils::getPwd() + "/ClientFolder_" + to_string(threadNum)).c_str(), 0777);

    // Create Socket
    if((clientSocket = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
        printf("Socket not created \n");
        pthread_exit(NULL);
        return nullptr;
    }
    ipOfServer.sin_family = AF_INET;     // Ipv4
    ipOfServer.sin_port = htons(port); // Port provided by user
    ipOfServer.sin_addr.s_addr = inet_addr(LOCALHOST); // Localhost=127.0.0.1

    // Connect to server
    if((n = connect(clientSocket, (struct sockaddr *)&ipOfServer, sizeof(ipOfServer)))<0)
    {
        printf("Connection failed due to port and ip problems\n");
        pthread_exit(NULL);
        return nullptr;
    }

    // Receive the file list from server
    TransferUtils::receiveFile(tempString, clientSocket);

    // Deserialize the files list and store in f
    SerializationUtils::deserializeFileList(tempString, f);
    threadData->f = &f;
    
    // If automated we only download 10 files and based on value of n, download then sequentially or parallely
    if(threadData->automated){
        // sequential
        n = 1;
        // Parallel
        if(threadData->automated==2)
            n = 2;

        for(int i = 0; i< f.files.size() && i <10;i++){
            files.push_back(i+1);
        }
    }
    else{
        // Print out the files in folder
        cout << "Files in the folder: " << endl;
        for(int i=0; i<f.numFiles; i++){
            cout << '\t' << i+1 << '\t' << f.files[i] << endl;
        }
        cout << endl;
        cout << "Which files do you need?" << endl;
        cout << "Input file numbers with spaces in between." << endl;

        // Read input and store file Numbers in files list
        while(1){
            tempString = "";
            getline(cin, tempString);
            istringstream s(tempString);
            while(getline(s, tempString, ' ')){
                n = stoi(tempString);
                files.push_back(n);
            }
            isValid = true;
            for(int i = 0; i< files.size(); i++){
                if(files[i] <=0 || files[i]>f.numFiles){
                    isValid = false;
                    cout << "Incorrect entry: " << files[i] << " Try again." << endl;
                    break;
                }
            }
            if(isValid) // If user input is valid then break
                break;
        }

        // Ask user whether to download then sequentially or parallely
        cout << "How do you want to download the files?" << endl;
        cout << "1: Serially, 2: Parallelly" << endl;

        // Read input from user
        while(1){
            cin >> n;
            if(n==1 || n==2){
                break;
            }
            else{
                cout << "Invalid input. Try again." << endl;
            }
        }
    }
    
    // Send the number of files to download to server
    TransferUtils::sendSize(files.size(), clientSocket);
    // Also send an indicator whether to download files serially or parallely
    TransferUtils::sendSize(n, clientSocket);

    // Instantiate some variables to validate Md5's and to determine which files to redownload
    string oldMd5 = "";
    string newMd5 = "";
    vector<int> filesToRedownload;
    struct stat newFileStat;
    bool redownload = true;
    clock_t beginTime;

    // This is the condition for sequential downloads
    if(n==1){
        while(!files.empty() && redownload){    // Infinite while loop till all files are download and redownload is not stopped
            redownload = false;
            beginTime = clock();
            for(int i: files){
                filesToRedownload = vector<int>{};
                
                TransferUtils::sendSize(i, clientSocket);   // Send the file Number for the file to download. This file number corresponds to what was shown in the UI
                oldMd5 = f.md5[i-1];
                cout << "Md5 for old file: " << oldMd5 << endl;
                cout << "Sent file number to server" << endl;
                cout << "Receiving file..." << endl;
                
                // This would be the new location of the file
                newFileLocation = FileUtils::getPwd() + "/ClientFolder_"+to_string(threadNum)+"/" + f.files[i-1].substr(f.directory.size()+1);

                // Receive custom file is used to receive the file in a custom size per packet(PACKET_DATA)
                TransferUtils::receiveCustomFile(newFileLocation, clientSocket);

                // Get the stats for the new file which would allow us to compare Md5
                newFileStat = FileUtils::getFileStat(newFileLocation);
                newMd5 = FileUtils::getMd5ForFile(newFileLocation, newFileStat.st_size);

                // If the Md5's don't math then add the file number to files to redownload
                cout << "Md5 for new file: " << newMd5 << endl;
                if(oldMd5.compare(newMd5) !=0){
                    filesToRedownload.push_back(i);
                    cout << "Md5's don't match for file: " << newFileLocation << endl;
                }
            }

            // Also log the time required to receive all the files
            cout << "Received files in: " << (float)(clock()-beginTime)/CLOCKS_PER_SEC << "  seconds" << endl;
            files = filesToRedownload;

            // If the client is automated we only log the number of files for which Md5's didn't match. We never redownload files
            if(threadData->automated){
                TransferUtils::printToFile(to_string(files.size()), "error.txt");
                //Donot redownload files for automated
                files = vector<int>{};
            }
            // This is logic for redownloads
            if(!files.empty()){
                // Get input from user whether to redownload
                cout << "Md5's didn't match for: " << files.size() << " files." << endl;
                cout << "Do you want to redownload?" << endl;
                cout << "1: Yes, 0: No" << endl;
                cin >> n;

                // If redownload we again send back the number of files to download and start the whole process again
                // We also remove the old files before redownload
                if(n==1){
                    // Resend number of files
                    TransferUtils::sendSize(files.size(), clientSocket); // send the number of files
                    redownload = true;
                    cout << "Redownloading: " << endl;
                    // Remove the oldd files
                    for(int i: files){
                        newFileLocation = FileUtils::getPwd() + "/ClientFolder_"+to_string(threadNum)+"/" + f.files[i-1].substr(f.directory.size()+1);
                        remove(newFileLocation.c_str());
                        cout << newFileLocation << endl;
                    }
                }
                else{
                    // Resend number of files as 0 if user donot want to redownload files
                    TransferUtils::sendSize(0, clientSocket);
                    cout << "Cancelling redownload." << endl;
                    break;
                }
            }
            else{
                // Resend number of files as 0 if all files were downloaded correctly and no redownloads required.
                TransferUtils::sendSize(0, clientSocket);
            }
        }
    }
    else{
        // If user wants to download files parallely
        while(!files.empty()){
            filesToRedownload = vector<int>{};
            // Method to receive files sent from server using multiple threads
            vector<FileUtils::FileInfo> fileInfo = TransferUtils::receiveCustomFilesMultithreaded(f, files, clientSocket, threadNum);
            
            // After download is complete compare the new Md5's to the old ones and polulat those file numbers in filesToDownload list
            for(FileUtils::FileInfo fInfo: fileInfo){
                oldMd5 = fInfo.fileMd5;
                newFileStat = FileUtils::getFileStat(fInfo.fileName);
                newMd5 = FileUtils::getMd5ForFile(fInfo.fileName, newFileStat.st_size);
                cout << "Old Md5: " << oldMd5 << endl;
                cout << "New Md5: " << newMd5 << endl;
                if(oldMd5.compare(newMd5) !=0){
                    filesToRedownload.push_back(files[fInfo.fileIdx]);
                    cout << "Md5's don't match for file: " << fInfo.fileName << endl;
                }
            }

            // Update the files list
            files = vector<int>(filesToRedownload.begin(), filesToRedownload.end());

            // If this an automated client then donot redownload files. Just log the number of files for which Md5's didnot match
            if(threadData->automated){
                TransferUtils::printToFile(to_string(files.size()), "error.txt");
                //Donot redownload files for automated
                files = vector<int>{};
            }

            // This is redownload logic. which is similar to tthat for sequential downloads.
            // We ask user whther to redownload and then send the number of files to redownload back to server
            if(!files.empty()){
                cout << "Md5's didn't match for: " << files.size() << " files." << endl;
                cout << "Do you want to redownload?" << endl;
                cout << "1: Yes, 0: No" << endl;
                cin >> n;
                if(n==1){
                    // Resend number of files to download again
                    TransferUtils::sendSize(files.size(), clientSocket);
                    redownload = true;
                    cout << "Redownloading: " << endl;

                    // Remove old files for which md5's didn't match
                    for(int i: files){
                        newFileLocation = FileUtils::getPwd() + "/ClientFolder_"+to_string(threadNum)+"/" + f.files[i-1].substr(f.directory.size()+1);
                        remove(newFileLocation.c_str());
                        cout << newFileLocation << endl;
                    }
                }
                else{
                    // Resend number of files as 0 if user selected not to re download files
                    TransferUtils::sendSize(0, clientSocket);
                    cout << "Cancelling redownload." << endl;
                    break;
                }
            }
            else{
                // Resend number of files as 0 if the md5's match for all files
                TransferUtils::sendSize(0, clientSocket);
            }
        }
    }

    // Finally exit the thread
    pthread_exit(NULL);
    return nullptr;
}