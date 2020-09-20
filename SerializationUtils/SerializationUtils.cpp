#include "SerializationUtils.h"
#include <algorithm>

string SerializationUtils::serializeFileList(FileList& f){
    string result = "";
    result+=to_string(f.numFiles) + "|";
    result+=f.directory + "|";
    for(string& s: f.files){
        result+=s+"|";
    }
    return result;
}

void SerializationUtils::deserializeFileList(string fileList, FileList& f){
    istringstream ss(fileList);
    string temp;
    int i = 0;
    while(getline(ss, temp, '|')){
        if(i==0){
            f.numFiles = stoi(temp);
        }
        else if(i==1){
            f.directory = temp;
        }
        else{
            f.files.push_back(temp);
        }
        i++;
    }
    f.files.pop_back();
}

void SerializationUtils::rtrim(string &s) {
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !isspace(ch) && ch!='0';
    }).base(), s.end());
}