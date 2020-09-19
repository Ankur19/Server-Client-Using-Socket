#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

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
};

