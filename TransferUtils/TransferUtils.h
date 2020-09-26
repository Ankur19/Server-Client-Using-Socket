#include <bits/stdc++.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include "../SerializationUtils/SerializationUtils.h"

using namespace std;

#define PACKET_SIZE (int)1024
#define NUMBER_SIZE (int)20

class TransferUtils{
    public:
        struct ThreadedFiles{
            vector<FILE*> files;
            vector<int> sizes;
            vector<FileUtils::FileInfo> fileInfo;
            int fileDescriptor;
        };
        static void printToFile(string s);
        //static void printSentPacket(string s);
        //static void printReceivedPacket(string s);
        static bool isPending(vector<int>& sizes);
        static void receiveFile(string& output, int fileDescriptor);
        static void receiveCustomFile(string& output, int fileDescriptor);
        static void sendFile(string& serializedFile, int clientDescriptor);
        static void sendSize(int size, int fileDescriptor);
        static int receiveSize(int fileDescriptor);
        static vector<int> getFileSizes(vector<int>& files, FileUtils::FileList* f);
        static FileUtils::FileInfo sendCustomFile(string fileLocation, int socketDescriptor);
        static void* sendCustomFileWithIndex(void* fileInfo);
        static void* receiveCustomFileWithIndex(void* fileInfo);
        static vector<FileUtils::FileInfo> sendCustomFilesMultithreaded(int& numFiles, FileUtils::FileList* f, int fileDescriptor);
        static vector<FileUtils::FileInfo> receiveCustomFilesMultithreaded(FileUtils::FileList& f, vector<int>& files, int fileDescriptor, int threadNum);
};