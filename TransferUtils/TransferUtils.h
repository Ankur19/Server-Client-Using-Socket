#include <bits/stdc++.h>
#include <sys/socket.h>
#include "../SerializationUtils/SerializationUtils.h"

using namespace std;

#define PACKET_SIZE 1024
#define SIZE_PACKET_SIZE 8

class TransferUtils{
    public:
        static void receiveFile(string& output, int fileDescriptor);
        static void sendFile(string& serializedFile, int clientDescriptor);
        static void sendSize(int size, int fileDescriptor);
        static int receiveSize(int fileDescriptor);
        static void sendCustomFile(string fileLocation, int socketDescriptor);
};