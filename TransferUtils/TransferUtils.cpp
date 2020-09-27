#include "TransferUtils.h"

void TransferUtils::receiveFile(string& output, int fileDescriptor){
    char dataReceiving[PACKET_SIZE];
    int size = 0, finalSize = 0, n = 0;
    output = "";

    size = receiveSize(fileDescriptor);
    finalSize = size;
    while(size>0)
    {
        memset(dataReceiving, '0', PACKET_SIZE);
        n = recv(fileDescriptor, dataReceiving, PACKET_SIZE, 0);
        size-=n;
        output+= string(dataReceiving);
    }
    SerializationUtils::rtrim(output);
}

void TransferUtils::receiveCustomFile(string& output, int fileDescriptor){
    char dataReceiving[PACKET_SIZE];
    int size = 0, finalSize = 0, n = 0;
    FILE* file = fopen(output.c_str(), "wb");

    size = receiveSize(fileDescriptor);
    finalSize = size;
    while(size>0)
    {
        memset(dataReceiving, '0', PACKET_SIZE);
        n = recv(fileDescriptor, dataReceiving, PACKET_SIZE, 0);
        fwrite(dataReceiving, sizeof(char), n, file);
        size-=n;
    }
    fclose(file);
}

void TransferUtils::sendSize(int size, int fileDescriptor){
    char sizePacket[NUMBER_SIZE];
    memset(sizePacket, ' ', NUMBER_SIZE);
    memcpy(sizePacket, to_string(size).c_str(), NUMBER_SIZE);
	send(fileDescriptor, sizePacket, NUMBER_SIZE, 0);
    return;
}

int TransferUtils::receiveSize(int fileDescriptor){
    string s;
    char sizePacket[NUMBER_SIZE];

    memset(sizePacket, ' ', NUMBER_SIZE);
    recv(fileDescriptor, sizePacket, NUMBER_SIZE, 0);
    return atoi(sizePacket);
}

void TransferUtils::sendFile(string& serializedFile, int clientDescriptor){
	int size = 0, n = 0;
    char dataSending[PACKET_SIZE];
	
	size = serializedFile.size();
	char* temp = (char* )malloc(size*sizeof(char));
	char* pointerToFree = temp;

    sendSize(size, clientDescriptor);
    memset(temp, ' ', size);
	memcpy(temp, serializedFile.c_str(), size);

	while(size > 0){
		memset(dataSending, ' ', PACKET_SIZE);
        memcpy(dataSending, temp, PACKET_SIZE);
        
        send(clientDescriptor, dataSending, PACKET_SIZE, 0);
        temp+=PACKET_SIZE;
        size-=PACKET_SIZE;
	}

	free(pointerToFree);
}

FileUtils::FileInfo TransferUtils::sendCustomFile(string fileLocation, int socketDescriptor){
    int size = 0, n = 0;
    char dataSending[PACKET_SIZE];
    FileUtils::FileInfo f;

    FILE* fd = fopen(fileLocation.c_str(), "rb");
    struct stat fileStat = FileUtils::getFileStat(fileLocation);
    f.fileName = fileLocation;

    size = fileStat.st_size;
    f.fileSize = fileStat.st_size;
    sendSize(size, socketDescriptor);
    
    while(size > 0){
        memset(dataSending, ' ', PACKET_SIZE);
        n = fread(dataSending, sizeof(char), PACKET_SIZE, fd);
        send(socketDescriptor, dataSending, n, 0);
        size-=n;
    }
    
    fclose(fd);
    return f;
}

vector<int> TransferUtils::getFileSizes(vector<int>& files, FileUtils::FileList* f){
    struct stat s;
    vector<int> sz{};
    for(int i = 0; i< files.size();i++){
        s = FileUtils::getFileStat(f->files[files[i]-1]);
        sz.push_back(s.st_size);
    }
    return sz;
}

vector<FileUtils::FileInfo> TransferUtils::sendCustomFilesMultithreaded(int& numFiles, FileUtils::FileList* f, int fileDescriptor){
    vector<int> files{};
    vector<int> sizes{};
    int threadIdx, n = 0;
    vector<FileUtils::FileInfo> fileInfo{};

    // Receive file numbers first
    cout << "Receiving fileNumbers for " << numFiles << " files" << endl;
    for(int i = 0; i<numFiles; i++){
        files.push_back(receiveSize(fileDescriptor));
    }
    
    // Then we send file sizes
    sizes = getFileSizes(files, f);
    cout << "Sending sizes for " << numFiles << " files" << endl;
    for(int i = 0; i<numFiles; i++){
        sendSize(sizes[i], fileDescriptor);
    }

    pthread_t threads[files.size()];
    fileInfo = vector<FileUtils::FileInfo>(files.size(), FileUtils::FileInfo());
    const clock_t beginTime = clock();
    //cout << "Creating thread for each file." << endl;
    for(int i = 0; i< files.size(); i++){
        fileInfo[i].fileName = f->files[files[i]-1];
        fileInfo[i].charFileName = (char*)malloc(sizeof(char)* fileInfo[i].fileName.size() + 1);
        memcpy(fileInfo[i].charFileName, fileInfo[i].fileName.c_str(), fileInfo[i].fileName.size());
        fileInfo[i].charFileName[fileInfo[i].fileName.size()] = '\0';
        fileInfo[i].fileMd5 = f->md5[files[i]-1];
        fileInfo[i].fileSize = sizes[i];
        fileInfo[i].fileIdx = i;
        fileInfo[i].fileDescriptor = fileDescriptor;
        //cout << "Creating thread: " << i+1 << endl;
        threadIdx = pthread_create(&threads[i], NULL, sendCustomFileWithIndex, (void*)&fileInfo[i]);
        if (threadIdx) {
			cout << "Error:unable to create thread," << threadIdx << endl;
			exit(-1);
		};
    }
    for(pthread_t pt: threads){
        pthread_join(pt, NULL);
    }

    TransferUtils::printToFile(to_string((float)(clock()-beginTime)/CLOCKS_PER_SEC));
    cout << "Sent files in: " << (float)(clock()-beginTime)/CLOCKS_PER_SEC << " milli seconds" << endl;

    for(int i = 0; i< files.size();i++){
        free(fileInfo[i].charFileName);
    }
    numFiles = receiveSize(fileDescriptor);
    cout << "Need to resend: " << numFiles  << " files." << endl;
    return fileInfo;
}

void* TransferUtils::sendCustomFileWithIndex(void* fileInfo){
    int size = 0, n = 0;
    char dataSending[PACKET_SIZE];
    FileUtils::FileInfo* fInfo = (FileUtils::FileInfo*)fileInfo;
    FILE* fd;

    fd = fopen(fInfo->charFileName, "rb");
    size = fInfo->fileSize;
    
    while(size > 0 && fd>0){
        memset(dataSending, ' ', PACKET_SIZE);
        memcpy(dataSending, to_string(fInfo->fileIdx).c_str(), NUMBER_SIZE);
        n = fread(dataSending+NUMBER_SIZE, sizeof(char), PACKET_SIZE-NUMBER_SIZE, fd);
        send(fInfo->fileDescriptor, dataSending, PACKET_SIZE, 0);
        size-=n;
    }
    fclose(fd);
    pthread_exit(NULL);
    return nullptr;
}

vector<FileUtils::FileInfo> TransferUtils::receiveCustomFilesMultithreaded(FileUtils::FileList& f, vector<int>& files, int fileDescriptor, int threadNum){
    vector<int> sizes;
    ThreadedFiles tf;
    char fileIdx[NUMBER_SIZE];
    vector<pthread_t>threads{};
    string newFileLocation;
    FILE* file;
    int threadIdx, n;
    char dataReceiving[PACKET_SIZE];
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    cout << "Sending file numbers. NumFiles: " << files.size() << endl;
    // Send file numbers
    for(int i = 0; i< files.size();i++){
        sendSize(files[i], fileDescriptor);
    }
    cout << "Getting file sizes." << endl;
    // Get file sizes
    for(int i = 0; i< files.size();i++){
        n = receiveSize(fileDescriptor);
        sizes.push_back(n);
    }
    cout << "sizes: ";
    for(int i: sizes){
        cout << i << ",";
    }
    cout << endl;

    tf.lock = lock;
    tf.fileDescriptor = fileDescriptor;
    for(int i = 0; i<files.size(); i++){
        FileUtils::FileInfo fInf;
        newFileLocation = FileUtils::getPwd() + "/ClientFolder_"+to_string(threadNum)+"/" + f.files[files[i]-1].substr(f.directory.size()+1);
        file = fopen(newFileLocation.c_str(), "wb");
        cout << "fd: " << file << " saving to: " << newFileLocation.c_str() << endl;
        tf.files.push_back(file);
        tf.sizes.push_back(sizes[i]);
        tf.fileInfo.push_back(fInf);
        tf.fileInfo[i].fileName = newFileLocation;
        tf.fileInfo[i].fileIdx = i;
        tf.fileInfo[i].fileSize = sizes[i];
        tf.fileInfo[i].fileMd5 = f.md5[files[i]-1];
        tf.locks.push_back(PTHREAD_MUTEX_INITIALIZER);
    }
    
    while(1){
        pthread_mutex_lock(&lock);
        if(!isPending(tf.sizes)){
            pthread_mutex_unlock(&lock);
            break;
        }
        pthread_mutex_unlock(&lock);
        pthread_t thread;
        pthread_create(&thread, NULL, saveToFile, (void*)&tf);
        threads.push_back(thread);
    }

    for(pthread_t pt: threads){
        pthread_join(pt, NULL);
    }

    for(FILE* i: tf.files){
        fclose(i);
    }
    cout << "Files received..!!" << endl;

    return tf.fileInfo;
}

void* TransferUtils::saveToFile(void* fileInfo){
    int size = 0, n = 0;
    char fileIdx[NUMBER_SIZE];
    char dataReceiving[PACKET_SIZE];
    ThreadedFiles* fInfo = (ThreadedFiles*)fileInfo;
    bool received = false;

    memset(dataReceiving, ' ', PACKET_SIZE);
    memset(fileIdx, ' ', NUMBER_SIZE);

    pthread_mutex_lock(&(fInfo->lock));
    if(isPending(fInfo->sizes)){
        recv(fInfo->fileDescriptor, dataReceiving, PACKET_SIZE, 0);
        memcpy(fileIdx, dataReceiving, NUMBER_SIZE);
        n = atoi(fileIdx);
        if(fInfo->sizes[n] > PACKET_SIZE-NUMBER_SIZE)
            fInfo->sizes[n]-=PACKET_SIZE-NUMBER_SIZE;
        received = true;
    }
    pthread_mutex_unlock(&(fInfo->lock));

    if(received){
        pthread_mutex_lock(&(fInfo->locks[n]));
        if(fInfo->sizes[n] <=PACKET_SIZE-NUMBER_SIZE){
            fwrite(dataReceiving+NUMBER_SIZE, sizeof(char), fInfo->sizes[n], fInfo->files[n]);
            fInfo->sizes[n] = 0;
        }
        else{
            fwrite(dataReceiving+NUMBER_SIZE, sizeof(char), PACKET_SIZE-NUMBER_SIZE, fInfo->files[n]);
        }
        pthread_mutex_unlock(&(fInfo->locks[n]));
    }
    pthread_exit(NULL);
    return nullptr;
}

bool TransferUtils::isPending(vector<int>& sizes){
    long long sum = (long long)accumulate(sizes.begin(), sizes.end(), 0);
    return sum>0;
}

void TransferUtils::printToFile(string s){
    ofstream file("time.txt", ios_base::app);
    file << s << endl;
    file.close();
}

/*void TransferUtils::printSentPacket(string s){
    ofstream file("serverPackets.txt", ios_base::app);
    file << s << endl;
    file.close();

}
void TransferUtils::printReceivedPacket(string s){
    ofstream file;
    file.open("clientPackets.txt", ios_base::app);
    file << s << endl;
    file.close();
}

void* TransferUtils::receiveCustomFileWithIndex(void* fileInfo){
    int size = 0, n = 0;
    char fileIdx[NUMBER_SIZE];
    time_t start, end;
    char dataReceiving[PACKET_SIZE];
    ThreadedFiles* fInfo = (ThreadedFiles*)fileInfo;
    string s;

    while(isPending(fInfo->sizes)){
        memset(dataReceiving, ' ', PACKET_SIZE);
        n = recv(fInfo->fileDescriptor, dataReceiving, PACKET_SIZE, 0);
        if(n>0){
            memcpy(fileIdx, dataReceiving, NUMBER_SIZE);
            s = string(fileIdx);
            SerializationUtils::rtrim(s);
            n = stoi(s);
            fInfo->sizes[n]-=PACKET_SIZE-NUMBER_SIZE;
            if(fInfo->sizes[n] <=0){
                fInfo->sizes[n] =0;
                fwrite(dataReceiving+NUMBER_SIZE, sizeof(char), fInfo->sizes[n] + (PACKET_SIZE-NUMBER_SIZE), fInfo->files[n]);
                pthread_exit(NULL);
            }
            else{
                fwrite(dataReceiving+NUMBER_SIZE, sizeof(char), PACKET_SIZE-NUMBER_SIZE, fInfo->files[n]);
            }
        }
    }
    pthread_exit(NULL);
    return nullptr;
}*/