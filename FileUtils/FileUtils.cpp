#include "FileUtils.h"

string FileUtils::getPwd(){
    return fs::current_path();
}

FileList* FileUtils::getFilesInDir(string dir){
    FileList* f = new FileList();
    
    f->directory = dir;

    for (const auto & entry : fs::directory_iterator(dir)){
        f->numFiles++;
        f->files.push_back(entry.path());
    }
    
    return f;
}