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

// Get the size of the file by its file descriptor
unsigned long FileUtils::getSizeByFd(int fd) {
    struct stat statbuf;
    if(fstat(fd, &statbuf) < 0) exit(-1);
    return statbuf.st_size;
}

string FileUtils::getMd5ForFile(int fileDescriptor){
    char* fileBuffer;
    unsigned char result[MD5_DIGEST_LENGTH];
    unsigned long fileSize = getSizeByFd(fileDescriptor);
    string s = "";
    fileBuffer = (char *)mmap(0, fileSize, PROT_READ, MAP_SHARED, fileDescriptor, 0);
    MD5((unsigned char*) fileBuffer, fileSize, result);
    munmap(fileBuffer, fileSize);
    for(int i = 0; i< MD5_DIGEST_LENGTH; i++){
        s+=string(1, result[i]);
    }
    return s;
}