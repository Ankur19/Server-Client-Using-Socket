#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include "TransferUtils/TransferUtils.h"

using namespace std;

#define LOCALHOST "127.0.0.1"
#define MAX_CLIENTS 8
#define DOWNLOADS_REMAINING (int)20


int main(int argc, const char* argv[])
{
    int clientSocket = 0,n = 0;
    char dataReceived[PACKET_SIZE];
    struct sockaddr_in ipOfServer;
    string tempString, newFileLocation;
    FileUtils::FileList f;
    vector<int> files;
    bool isValid;
    // Get port from main arguments
	int port = stoi(argv[1]);

    memset(dataReceived, '0' ,sizeof(dataReceived));

    if((clientSocket = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
        printf("Socket not created \n");
        return 1;
    }
 
    ipOfServer.sin_family = AF_INET;
    ipOfServer.sin_port = htons(port);
    ipOfServer.sin_addr.s_addr = inet_addr(LOCALHOST);
 
    if((n = connect(clientSocket, (struct sockaddr *)&ipOfServer, sizeof(ipOfServer)))<0)
    {
        printf("Connection failed due to port and ip problems\n");
        return 1;
    }

    TransferUtils::receiveFile(tempString, clientSocket);

    SerializationUtils::deserializeFileList(tempString, f);
    
    cout << "Files in the folder: " << endl;
    for(int i=0; i<f.numFiles; i++){
        cout << '\t' << i+1 << '\t' << f.files[i] << endl;
    }

    cout << endl;
    cout << "Which files do you need?" << endl;
    cout << "Input file numbers with spaces in between." << endl;
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
        if(isValid)
            break;
    }
    cout << "How do you want to download the files?" << endl;
    cout << "1: Serially, 2: Parallelly" << endl;
    while(1){
        cin >> n;
        if(n==1 || n==2){
            break;
        }
        else{
            cout << "Invalid input. Try again." << endl;
        }
    }
    // Number of Files
    TransferUtils::sendSize(files.size(), clientSocket);
    // Serially or parallely
    TransferUtils::sendSize(n, clientSocket);
    string oldMd5 = "";
    string newMd5 = "";
    int downloadsRemaining = DOWNLOADS_REMAINING;
    vector<int> filesToRedownload;
    struct stat newFileStat;
    if(n==1){
        while(!files.empty() && downloadsRemaining){
            for(int i: files){
                filesToRedownload = vector<int>{};
                
                TransferUtils::sendSize(i, clientSocket);
                oldMd5 = f.md5[i-1];
                cout << "Md5 for old file: " << oldMd5 << endl;
                cout << "Sent file number to server" << endl;
                cout << "Receiving file..." << endl;
                
                newFileLocation = FileUtils::getPwd() + "/ClientFolder/" + f.files[i-1].substr(f.directory.size()+1);
                TransferUtils::receiveCustomFile(newFileLocation, clientSocket);
                newFileStat = FileUtils::getFileStat(newFileLocation);

                newMd5 = FileUtils::getMd5ForFile(newFileLocation, newFileStat.st_size);
                cout << "Md5 for new file: " << newMd5 << endl;
                if(oldMd5.compare(newMd5) !=0){
                    remove(newFileLocation.c_str());
                    filesToRedownload.push_back(i);
                    cout << "Md5's don't match for file: " << newFileLocation << endl;
                    cout << "Downloads remaining: " << downloadsRemaining << endl;
                }
            }
            files = filesToRedownload;
            // Resend number of files
            TransferUtils::sendSize(files.size(), clientSocket);
            
            if(files.size()>0)
                downloadsRemaining--;
        }
    }
    else{
        while(!files.empty() && downloadsRemaining){
            filesToRedownload = vector<int>{};
            vector<FileUtils::FileInfo> fileInfo = TransferUtils::receiveCustomFilesMultithreaded(f, files, clientSocket);
            for(FileUtils::FileInfo fInfo: fileInfo){
                oldMd5 = fInfo.fileMd5;
                newFileStat = FileUtils::getFileStat(fInfo.fileName);
                newMd5 = FileUtils::getMd5ForFile(fInfo.fileName, newFileStat.st_size);
                cout << "Old Md5: " << oldMd5 << endl;
                cout << "New Md5: " << newMd5 << endl;
                if(oldMd5.compare(newMd5) !=0){
                    if(downloadsRemaining > 1)
                        remove(fInfo.fileName.c_str());
                    filesToRedownload.push_back(files[fInfo.fileIdx]);
                    cout << "Md5's don't match for file: " << fInfo.fileName << endl;
                    cout << "Downloads remaining: " << downloadsRemaining << endl;
                }
            }
            files = vector<int>(filesToRedownload.begin(), filesToRedownload.end());
            // Resend number of files
            TransferUtils::sendSize(files.size(), clientSocket);
            
            if(files.size()>0)
                downloadsRemaining--;
        }
    }
    close(clientSocket);
    return 0;
}