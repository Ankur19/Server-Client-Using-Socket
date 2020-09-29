#include <bits/stdc++.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include "../SerializationUtils/SerializationUtils.h"

using namespace std;

class TransferUtils{
    public:
        struct ThreadedFiles{
            vector<FILE*> files;
            vector<int> sizes;
            vector<FileUtils::FileInfo> fileInfo;
            int fileDescriptor;
            vector<pthread_mutex_t> locks;
            pthread_mutex_t lock;
        };
        static void printToFile(string s, string fileName);
        //static void printSentPacket(string s);
        //static void printReceivedPacket(string s);
        static bool isPending(vector<int>& sizes);
        static void receiveFile(string& output, int fileDescriptor);
        static void receiveCustomFile(string& output, int fileDescriptor);
        static void sendFile(string& serializedFile, int clientDescriptor);
        static void sendSize(int size, int fileDescriptor);
        static int receiveSize(int fileDescriptor);
        static vector<int> getFileSizes(vector<int>& files, FileUtils::FileList* f);
        static FileUtils::FileInfo sendCustomFile(string fileLocation, int socketDescriptor, int clientNumber);
        static void* sendCustomFileWithIndex(void* fileInfo);
        static void* saveToFile(void* fileInfo);
        static vector<FileUtils::FileInfo> sendCustomFilesMultithreaded(int& numFiles, FileUtils::FileList* f, int fileDescriptor, int clientNumber);
        static vector<FileUtils::FileInfo> receiveCustomFilesMultithreaded(FileUtils::FileList& f, vector<int>& files, int fileDescriptor, int threadNum);
};