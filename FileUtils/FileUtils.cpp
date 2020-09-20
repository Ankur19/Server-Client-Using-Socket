#include "FileUtils.h"

string FileUtils::getPwd(){
    return fs::current_path();
}

FileList* FileUtils::getFilesInDir(string dir){
    FileList* f = new FileList();
    int fptr;
    f->directory = dir;
    struct stat fileStat;
    for (const auto & entry : fs::directory_iterator(dir)){
        fptr = open(entry.path().c_str(), O_RDONLY);
        fileStat = getFileStat(fptr);
        if(!S_ISDIR(fileStat.st_mode)){
            f->numFiles++;
            f->files.push_back(entry.path());
            f->md5.push_back(getMd5ForFile(fptr, fileStat.st_size));
        }
        close(fptr);
    }
    return f;
}

// Get the size of the file by its file descriptor
struct stat FileUtils::getFileStat(int fd) {
    struct stat statbuf;
    if(fstat(fd, &statbuf) < 0) exit(-1);
    return statbuf;
}

string FileUtils::getMd5ForFile(int fileDescriptor, unsigned long fileSize){
    char* fileBuffer;
    unsigned char result[MD5_DIGEST_LENGTH];
    string s = "";
    char buf[32];
    fileBuffer = (char *)mmap(0, fileSize, PROT_READ, MAP_SHARED, fileDescriptor, 0);
    MD5((unsigned char*) fileBuffer, fileSize, result);
    munmap(fileBuffer, fileSize);
    for(int i = 0; i< MD5_DIGEST_LENGTH; i++){
        sprintf(buf, "%02x", result[i]);
        s.append(buf);
    }
    return s;
}