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

using namespace std;
namespace fs = filesystem;

class FileUtils{
    public:
        struct FileList{
            int numFiles;
            string directory;
            vector<string> files;
            vector<string> md5;
            FileList() : numFiles(0), directory(""), files(vector<string>{}), md5(vector<string>{}) {};
            FileList(int numberOfFiles, string dir, vector<string> fileList, vector<string> md5s) : numFiles(numberOfFiles), directory(dir), files(fileList), md5(md5s) {};
        };
        static string getPwd();
        static FileList* getFilesInDir(string dir);
        static void printMd5Sum(unsigned char* md);
        static struct stat getFileStat(int fd);
        static string getMd5ForFile(int fileDescriptor, unsigned long fileSize);
};

