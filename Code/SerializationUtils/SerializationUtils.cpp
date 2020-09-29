#include "SerializationUtils.h"
#include <algorithm>

// Serialize the file list by converting it to a readable string which can be sent over TCP
string SerializationUtils::serializeFileList(FileUtils::FileList& f){
    string result = "";
    result+=to_string(f.numFiles) + "|";
    result+=f.directory + "|";
    for(int i = 0; i<f.files.size(); i++){
        result+=f.files[i] + "|" + f.md5[i] + "|";
    }
    return result;
}

// Deserialize the file that was serialized using the serializeFileList method.
void SerializationUtils::deserializeFileList(string& fileList, FileUtils::FileList& f){
    istringstream ss(fileList);
    string temp = "";
    bool first = true;
    int i = 0;
    while(getline(ss, temp, '|')){
        if(i==0){
            f.numFiles = stoi(temp);
        }
        else if(i==1){
            f.directory = temp;
        }
        else{
            if(first){
                f.files.push_back(temp);
                first = false;
            }
            else{
                f.md5.push_back(temp);
                first = true;
            }
        }
        i++;
    }
    f.files.pop_back();
}

// Trim space and empty characters from right
void SerializationUtils::rtrim(string &s) {
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !isspace(ch) && ch != '0';
    }).base(), s.end());
}