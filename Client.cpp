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
#define PORT_NUMBER 50015
#define MAX_CLIENTS 8


int main()
{
    int clientSocket = 0,n = 0;
    char dataReceived[PACKET_SIZE];
    struct sockaddr_in ipOfServer;
    string tempString, newFileLocation;
    FileUtils::FileList f;
    vector<int> files;
    bool isValid;
    
    memset(dataReceived, '0' ,sizeof(dataReceived));

    if((clientSocket = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
        printf("Socket not created \n");
        return 1;
    }
 
    ipOfServer.sin_family = AF_INET;
    ipOfServer.sin_port = htons(PORT_NUMBER);
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

    TransferUtils::sendSize(files.size(), clientSocket);
    TransferUtils::sendSize(n, clientSocket);

    if(n==1){
        for(int i: files){
            TransferUtils::sendSize(i, clientSocket);
            cout << "Md5 for old file: " << f.md5[i-1] << endl;
            cout << "Sent file number to server" << endl;
            cout << "Receiving file..." << endl;
            
            newFileLocation = FileUtils::getPwd() + "/ClientFolder/" + f.files[i-1].substr(f.directory.size()+1);
            TransferUtils::receiveCustomFile(newFileLocation, clientSocket);
            
            struct stat newFileStat = FileUtils::getFileStat(newFileLocation);
            
            cout << "Md5 for new file: " << FileUtils::getMd5ForFile(newFileLocation, newFileStat.st_size) << endl;
        }
    }
    else{
        vector<FileUtils::FileInfo> fileInfo = TransferUtils::receiveCustomFilesMultithreaded(f, files, clientSocket);
    }

    return 0;
}