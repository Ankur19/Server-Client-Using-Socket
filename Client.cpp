#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include "TransferUtils/TransferUtils.h"

using namespace std;

void* connectToServer(void* tD);

struct ThreadData{
	int clientDescriptor;
    int port;
	string serializedFile;
	vector<FileUtils::FileInfo> sentFiles;
	FileUtils::FileList* f;
    int automated;
    int threadNum;
	ThreadData(): clientDescriptor(0), serializedFile(""), sentFiles(vector<FileUtils::FileInfo>{}), f(nullptr), threadNum(0), port(0), automated(0) {}
};

int main(int argc, const char* argv[])
{
    int clientSocket = 0,n = 0;
    
    int port = stoi(argv[1]);
    int folderIndex = stoi(argv[2]);
    int automated = stoi(argv[3]);

    pthread_t threads[1];
    vector<ThreadData*> threadDataList(1, new ThreadData());

    for(int i = 0; i<1;i++){
        threadDataList[i]->port = port;
        threadDataList[i]->threadNum = folderIndex;
        threadDataList[i]->automated = automated;
        n = pthread_create(&threads[i], NULL, connectToServer, (void*)threadDataList[i]);
        if (n) {
			cout << "Error:unable to create thread," << n << endl;
			exit(-1);
		};
    }
    for(pthread_t& pt: threads){
        pthread_join(pt, NULL);
    }
    return 0;
}

void* connectToServer(void* tD){
    ThreadData* threadData = (ThreadData*)tD;
    
    int clientSocket, threadNum, n = 0, port;
    string tempString, newFileLocation;
    vector<int> files{};
    bool isValid;
    FileUtils::FileList f;

    threadNum = threadData->threadNum;
    struct sockaddr_in ipOfServer;
    port = threadData->port;
    n = mkdir((FileUtils::getPwd() + "/ClientFolder_" + to_string(threadNum)).c_str(), 0777);

    if((clientSocket = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
        printf("Socket not created \n");
        pthread_exit(NULL);
        return nullptr;
    }
    ipOfServer.sin_family = AF_INET;
    ipOfServer.sin_port = htons(port);
    ipOfServer.sin_addr.s_addr = inet_addr(LOCALHOST);

    if((n = connect(clientSocket, (struct sockaddr *)&ipOfServer, sizeof(ipOfServer)))<0)
    {
        printf("Connection failed due to port and ip problems\n");
        pthread_exit(NULL);
        return nullptr;
    }

    TransferUtils::receiveFile(tempString, clientSocket);

    SerializationUtils::deserializeFileList(tempString, f);
    threadData->f = &f;
    
    if(threadData->automated){
        //Always parallel
        n = 1;
        for(int i = 0; i< f.files.size();i++){
            files.push_back(i+1);
        }
    }
    else{
        cout << "Files in the folder: " << endl;
        for(int i=0; i<f.numFiles; i++){
            cout << '\t' << i+1 << '\t' << f.files[i] << endl;
        }

        cout << endl;
        cout << "Which files do you need?" << endl;
        cout << "Input file numbers with spaces in between." << endl;
        while(1){
            tempString = "";
            getline(cin, tempString);
            istringstream s(tempString);
            while(getline(s, tempString, ' ')){
                n = stoi(tempString);
                files.push_back(n);
            }
            isValid = true;
            for(int i = 0; i< files.size(); i++){
                if(files[i] <=0 || files[i]>f.numFiles){
                    isValid = false;
                    cout << "Incorrect entry: " << files[i] << " Try again." << endl;
                    break;
                }
            }
            if(isValid)
                break;
        }
        cout << "How do you want to download the files?" << endl;
        cout << "1: Serially, 2: Parallelly" << endl;
        while(1){
            cin >> n;
            if(n==1 || n==2){
                break;
            }
            else{
                cout << "Invalid input. Try again." << endl;
            }
        }
    }
    
    // Number of Files
    TransferUtils::sendSize(files.size(), clientSocket);
    // Serially or parallely
    TransferUtils::sendSize(n, clientSocket);
    string oldMd5 = "";
    string newMd5 = "";
    vector<int> filesToRedownload;
    struct stat newFileStat;
    bool redownload = true;
    if(n==1){
        while(!files.empty() && redownload){
            redownload = false;
            for(int i: files){
                filesToRedownload = vector<int>{};
                
                TransferUtils::sendSize(i, clientSocket);
                oldMd5 = f.md5[i-1];
                cout << "Md5 for old file: " << oldMd5 << endl;
                cout << "Sent file number to server" << endl;
                cout << "Receiving file..." << endl;
                
                newFileLocation = FileUtils::getPwd() + "/ClientFolder_"+to_string(threadNum)+"/" + f.files[i-1].substr(f.directory.size()+1);
                TransferUtils::receiveCustomFile(newFileLocation, clientSocket);
                newFileStat = FileUtils::getFileStat(newFileLocation);

                newMd5 = FileUtils::getMd5ForFile(newFileLocation, newFileStat.st_size);
                cout << "Md5 for new file: " << newMd5 << endl;
                if(oldMd5.compare(newMd5) !=0){
                    filesToRedownload.push_back(i);
                    cout << "Md5's don't match for file: " << newFileLocation << endl;
                }
            }
            files = filesToRedownload;
            
            if(!files.empty()){
                cout << "Md5's didn't match for: " << files.size() << " files." << endl;
                cout << "Do you want to redownload?" << endl;
                cout << "1: Yes, 0: No" << endl;
                cin >> n;
                if(n==1){
                    // Resend number of files
                    TransferUtils::sendSize(files.size(), clientSocket);
                    redownload = true;
                    cout << "Redownloading: " << endl;
                    for(int i: files){
                        newFileLocation = FileUtils::getPwd() + "/ClientFolder_"+to_string(threadNum)+"/" + f.files[i-1].substr(f.directory.size()+1);
                        remove(newFileLocation.c_str());
                        cout << newFileLocation << endl;
                    }
                }
                else{
                    // Resend number of files
                    TransferUtils::sendSize(0, clientSocket);
                    cout << "Cancelling redownload." << endl;
                    break;
                }
            }
            else{
                // Resend number of files
                TransferUtils::sendSize(0, clientSocket);
            }
        }
    }
    else{
        while(!files.empty()){
            filesToRedownload = vector<int>{};
            vector<FileUtils::FileInfo> fileInfo = TransferUtils::receiveCustomFilesMultithreaded(f, files, clientSocket, threadNum);
            for(FileUtils::FileInfo fInfo: fileInfo){
                oldMd5 = fInfo.fileMd5;
                newFileStat = FileUtils::getFileStat(fInfo.fileName);
                newMd5 = FileUtils::getMd5ForFile(fInfo.fileName, newFileStat.st_size);
                cout << "Old Md5: " << oldMd5 << endl;
                cout << "New Md5: " << newMd5 << endl;
                if(oldMd5.compare(newMd5) !=0){
                    filesToRedownload.push_back(files[fInfo.fileIdx]);
                    cout << "Md5's don't match for file: " << fInfo.fileName << endl;
                }
            }
            files = vector<int>(filesToRedownload.begin(), filesToRedownload.end());

            if(!files.empty()){
                cout << "Md5's didn't match for: " << files.size() << " files." << endl;
                cout << "Do you want to redownload?" << endl;
                cout << "1: Yes, 0: No" << endl;
                cin >> n;
                if(n==1){
                    // Resend number of files
                    TransferUtils::sendSize(files.size(), clientSocket);
                    redownload = true;
                    cout << "Redownloading: " << endl;
                    for(int i: files){
                        newFileLocation = FileUtils::getPwd() + "/ClientFolder_"+to_string(threadNum)+"/" + f.files[i-1].substr(f.directory.size()+1);
                        remove(newFileLocation.c_str());
                        cout << newFileLocation << endl;
                    }
                }
                else{
                    // Resend number of files
                    TransferUtils::sendSize(0, clientSocket);
                    cout << "Cancelling redownload." << endl;
                    break;
                }
            }
            else{
                // Resend number of files
                TransferUtils::sendSize(0, clientSocket);
            }
        }
    }
    pthread_exit(NULL);
    return nullptr;
}