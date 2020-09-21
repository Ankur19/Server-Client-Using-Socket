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
#include "SerializationUtils/SerializationUtils.h"

using namespace std;

#define PACKET_SIZE 1024
#define LOCALHOST "127.0.0.1"
#define PORT_NUMBER 50015
#define MAX_CLIENTS 8

int main()
{
    int clientSocket = 0,n = 0;
    char dataReceived[PACKET_SIZE];
    struct sockaddr_in ipOfServer;
    string tempString;
    
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
    tempString = "";
    FileList f;
    while((n = read(clientSocket, dataReceived, sizeof(dataReceived))) > 0)
    {
        tempString+= string(dataReceived);
    }

    SerializationUtils::deserializeFileList(tempString, f);
    
    cout << "Files in the folder: " << endl;
    for(int i=0; i<f.numFiles; i++){
        cout << '\t' << i+1 << '\t' << f.files[i] << endl;
    }

    cout << endl;
    cout << "Which file number do you need?" << endl;
    while(true){
        cin >> n;
        if(n <=0 || n>f.numFiles){
            cout << "Incorrect entry. Try again." << endl;
        }
        else{
            break;
        }
    }
    cout << "Md5 for file " << f.files[n-1] << ": " << f.md5[n-1] << endl;

    tempString = "";
    memset(dataReceived, '0' ,sizeof(dataReceived));
    while((n = read(clientSocket, dataReceived, sizeof(dataReceived))) > 0)
    {
        tempString+= string(dataReceived);
    }

    SerializationUtils::rtrim(tempString);
    
    ofstream myfile("/ClientFolder/" + f.files[n-1].substr(f.directory.size()+1));
    myfile << tempString;
    myfile.close();

    n = open(("/ClientFolder/" + f.files[n-1].substr(f.directory.size()+1)).c_str(), O_RDONLY);
    struct stat newFileStat = FileUtils::getFileStat(n);
    cout << "Md5 for new file: " << FileUtils::getMd5ForFile(n, newFileStat.st_size) << endl;
    close(n);
    return 0;
}