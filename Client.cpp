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
    int CreateSocket = 0,n = 0;
    char dataReceived[PACKET_SIZE];
    struct sockaddr_in ipOfServer;
    string tempString;
    
    memset(dataReceived, '0' ,sizeof(dataReceived));

    if((CreateSocket = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
        printf("Socket not created \n");
        return 1;
    }
 
    ipOfServer.sin_family = AF_INET;
    ipOfServer.sin_port = htons(PORT_NUMBER);
    ipOfServer.sin_addr.s_addr = inet_addr(LOCALHOST);
 
    if((n = connect(CreateSocket, (struct sockaddr *)&ipOfServer, sizeof(ipOfServer)))<0)
    {
        printf("Connection failed due to port and ip problems\n");
        return 1;
    }
    tempString = "";
    FileList f;
    while((n = read(CreateSocket, dataReceived, sizeof(dataReceived))) > 0)
    {
        tempString+= string(dataReceived);
    }

    SerializationUtils::deserializeFileList(tempString, f);
    for(string md: f.md5)
        cout << md << endl;
    cout << "Files in the folder: " << endl;
    for(int i=0; i<f.numFiles; i++){
        cout << '\t' << i+1 << '\t' << f.files[i] << endl;
    }

    cout << endl;
    cout << "Which file number do you need? " << endl;
    cin >> n;
    cout << n << endl;
    /*
    ofstream myfile("ret.txt");
    myfile << tempString;
    myfile.close();*/
 
    if( n < 0)
    {
        printf("Standard input error \n");
    }
 
    return 0;
}