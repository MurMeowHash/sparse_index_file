#include "utils.h"
#include "../Error/Error.h"
#include "../Random/Random.h"

std::streamoff getFileSize(std::fstream &file) {
    std::streamoff initialOffset = file.tellg();
    file.seekg(0, std::ios::end);
    std::streamoff fileSize = file.tellg();
    file.seekg(initialOffset);
    return fileSize;
}

void openFile(std::fstream &stream, const char *path, std::ios::openmode mode) {
    stream.open(path, mode);
    if(!stream) {
        Error::fallWithError("FAILED_TO_CREATE_FILE");
    }
}

std::string genRandomString(uint length) {
    std::string result;
    for(uint i = 0; i < length; i++) {
        result += static_cast<char>(Random::next(97, 122));
    }
    return result;
}