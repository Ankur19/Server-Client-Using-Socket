#include "../FileUtils/FileUtils.h"

class SerializationUtils{
    public:
        static string serializeFileList(FileUtils::FileList& f);
        static void deserializeFileList(string& fileList, FileUtils::FileList& f);
        static void rtrim(string &s);
};