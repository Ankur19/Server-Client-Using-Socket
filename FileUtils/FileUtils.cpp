#include "FileUtils.h"

string FileUtils::getPwd(){
    return fs::current_path();
}

FileUtils::FileList* FileUtils::getFilesInDir(string dir){
    FileList* f = new FileList();
    f->directory = dir;
    struct stat fileStat;
    for (const auto & entry : fs::directory_iterator(dir)){
        fileStat = getFileStat(entry.path());
        if(!S_ISDIR(fileStat.st_mode)){
            f->numFiles++;
            f->files.push_back(string(entry.path()));
            f->md5.push_back(getMd5ForFile(entry.path(), fileStat.st_size));
        }
    }
    return f;
}

// Get the size of the file by its file descriptor
struct stat FileUtils::getFileStat(string fileLocation) {
    struct stat statbuf;
    int fd = open(fileLocation.c_str(), O_RDONLY);
    if(fstat(fd, &statbuf) < 0) exit(-1);
    close(fd);
    return statbuf;
}

string FileUtils::getMd5ForFile(string fileLocation, unsigned long fileSize){
    char* fileBuffer;
    unsigned char result[MD5_DIGEST_LENGTH];
    string s = "";
    char buf[32];
    int fileDescriptor = open(fileLocation.c_str(), O_RDONLY);

    fileBuffer = (char *)mmap(0, fileSize, PROT_READ, MAP_SHARED, fileDescriptor, 0);
    MD5((unsigned char*) fileBuffer, fileSize, result);
    munmap(fileBuffer, fileSize);
    for(int i = 0; i< MD5_DIGEST_LENGTH; i++){
        sprintf(buf, "%02x", result[i]);
        s.append(buf);
    }
    close(fileDescriptor);
    return s;
}