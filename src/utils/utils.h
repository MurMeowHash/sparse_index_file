#pragma once

#include <fstream>

#define NODISCARD [[nodiscard]]
#define UNUSED [[maybe_unused]]

using uint = unsigned int;

NODISCARD std::streamoff getFileSize(std::fstream &file);
void openFile(std::fstream &stream, const char *path, std::ios::openmode mode);
NODISCARD std::string genRandomString(uint length);