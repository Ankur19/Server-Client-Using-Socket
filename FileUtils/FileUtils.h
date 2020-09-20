#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <sys/stat.h>
#include <sys/mman.h>
#include <openssl/md5.h>

using namespace std;
namespace fs = filesystem;

struct FileList{
    int numFiles;
    string directory;
    vector<string> files;
    FileList() : numFiles(0), directory(""), files(vector<string>{}) {};
    FileList(int numberOfFiles, string dir, vector<string> fileList) : numFiles(numberOfFiles), directory(dir), files(fileList) {};
};

class FileUtils{
    public:
        static string getPwd();
        static FileList* getFilesInDir(string dir);
        static void printMd5Sum(unsigned char* md);
        static unsigned long getSizeByFd(int fd);
        static string getMd5ForFile(int fileDescriptor);
};

