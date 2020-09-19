#include "../FileUtils/FileUtils.h"

class SerializationUtils{
    public:
        static string serializeFileList(FileList& f);
        static void deserializeFileList(string fileList, FileList& f);
};