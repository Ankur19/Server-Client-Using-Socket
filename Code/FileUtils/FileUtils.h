#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <sys/stat.h>
#include <sys/mman.h>
#include <openssl/md5.h>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include "../VARIABLES.h"

using namespace std;
namespace fs = filesystem;

class FileUtils{
    public:
        typedef struct FileList{
            int numFiles;
            string directory;
            vector<string> files;
            vector<string> md5;
            vector<int> sizes;
            FileList() : numFiles(0), directory(""), files(vector<string>{}), md5(vector<string>{}), sizes(vector<int>{}) {};
            FileList(int numberOfFiles, string dir, vector<string> fileList, vector<string> md5s) : numFiles(numberOfFiles), directory(dir), files(fileList), md5(md5s), sizes(vector<int>{}) {};
        } FileList;
        typedef struct FileInfo{
            string fileName;
            int fileSize;
            char* charFileName;
            string fileMd5;
            int fileIdx;
            double timeToSend;
            int fileDescriptor;
            FileInfo(): fileName(""), fileSize(0), charFileName(nullptr), fileMd5(""), fileIdx(-1), timeToSend(0), fileDescriptor(-1) {}
        } FileInfo;
        static string getPwd();
        static FileList* getFilesInDir(string dir);
        static void printMd5Sum(unsigned char* md);
        static struct stat getFileStat(string fileLocation);
        static string getMd5ForFile(string fileLocation, unsigned long fileSize);
};

