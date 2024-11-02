#include "SparseIndexFile.h"
#include <iostream>
#include <filesystem>
#include <limits>

const uint SparseIndexFile::INFINITE_THRESHOLD{std::numeric_limits<uint>::max()};

SparseIndexFile::SparseIndexFile(uint blocksCount, uint blocksSize, uint increaseFactor)
: blocksCount{blocksCount}, blocksSize{blocksSize}, increaseFactor{increaseFactor}, dataBlocks(blocksCount) {
    indexAreaPath = INDEX_FILES_DIRECTORY + INDEX_AREA_NAME + BIN_EXTENSION;
    dataBlockName = INDEX_FILES_DIRECTORY + DATA_BLOCK_NAME;
    indexFilePropertiesPath = INDEX_FILES_DIRECTORY + INDEX_FILE_PROPERTIES_NAME + BIN_EXTENSION;
    tryLoadProperties();
    openIndexArea();
    openDataBlocks();
}

void SparseIndexFile::openIndexArea() {
    indexArea.open(indexAreaPath.c_str(), OPEN_MODE);
    if(!indexArea) {
        openFile(indexArea, indexAreaPath.c_str(), CREATE_MODE);
        rebuildIndexArea();
    }
}

void SparseIndexFile::openDataBlocks() {
    for(uint i = 0; i < dataBlocks.size(); i++) {
        std::string dataBlockPath = dataBlockName + std::to_string(i).append(BIN_EXTENSION);
        dataBlocks[i].open(dataBlockPath.c_str(), OPEN_MODE);
        if(!dataBlocks[i]) {
            openFile(dataBlocks[i], dataBlockPath.c_str(), CREATE_MODE);
        }
    }
}

Table<uint, uint> SparseIndexFile::rebuildIndexArea() {
    Table<uint, uint> adjustedIndexArea(blocksCount);
    indexArea.seekp(0);
    for(uint i = 0; i < blocksCount; i++) {
        uint blockUpperIndex = blocksSize * i;
        indexArea.write(reinterpret_cast<const char *>(&blockUpperIndex), sizeof(uint));
        indexArea.write(reinterpret_cast<const char *>(&i), sizeof(uint));
        adjustedIndexArea[i] = std::make_pair(blockUpperIndex, i);
    }
    return adjustedIndexArea;
}

Table<uint, uint> SparseIndexFile::getIndexArea() {
    indexArea.seekg(0);
    Table<uint, uint> indexAreaTable(blocksCount);
    for(uint i = 0; i < blocksCount; i++) {
        indexArea.read(reinterpret_cast<char *>(&indexAreaTable[i].first), sizeof(uint));
        indexArea.read(reinterpret_cast<char *>(&indexAreaTable[i].second), sizeof(uint));
    }
    return indexAreaTable;
}

uint SparseIndexFile::retrieveDataBlockIndex(uint key) {
    uint indexTablePosition = std::min(key / blocksSize, blocksCount - 1);
    auto indexAreaTable = getIndexArea();
    return indexAreaTable[indexTablePosition].second;
}

TableRow<uint, std::string> SparseIndexFile::readDataUnit(std::fstream &dataBlock) {
    TableRow<uint, std::string> dataUnit;
    uint unitLength;
    dataBlock.read(reinterpret_cast<char *>(&dataUnit.first), sizeof(uint));
    dataBlock.read(reinterpret_cast<char *>(&unitLength), sizeof(uint));
    dataUnit.second.resize(unitLength);
    dataBlock.read(reinterpret_cast<char *>(dataUnit.second.data()), unitLength);
    return dataUnit;
}

void SparseIndexFile::writeDataUnit(std::fstream &dataBlock, const TableRow<uint, std::string> &dataUnit) {
    dataBlock.write(reinterpret_cast<const char *>(&dataUnit.first), sizeof(uint));
    const std::string &dataUnitValue = dataUnit.second;
    uint dataUnitValueLength = dataUnitValue.size();
    dataBlock.write(reinterpret_cast<const char *>(&dataUnitValueLength), sizeof(uint));
    dataBlock.write(reinterpret_cast<const char *>(dataUnitValue.data()), dataUnitValueLength);
}

Table<uint, std::string> SparseIndexFile::getDataBlockContent(uint dataBlockIndex) {
    std::fstream &dataBlock = dataBlocks[dataBlockIndex];
    auto initialOffset = dataBlock.tellg();
    dataBlock.seekg(0);
    Table<uint, std::string> dataBlockContent;
    bool reachedEOF{false};
    while(!reachedEOF) {
        TableRow<uint, std::string> dataUnit;
        dataUnit = readDataUnit(dataBlock);
        reachedEOF = dataBlock.eof();
        if(!reachedEOF) {
            dataBlockContent.emplace_back(dataUnit);
        }
    }
    dataBlock.clear();
    dataBlock.seekg(initialOffset);
    return dataBlockContent;
}

bool SparseIndexFile::insertDataUnit(const TableRow<uint, std::string> &dataUnit, uint dataBlockIndex,
                                     const Table<uint, std::string> &dataBlockContent, uint position,
                                     bool inclusively) {
    std::fstream &dataBlock = dataBlocks[dataBlockIndex];
    std::streamoff offset{0};
    uint i;
    for(i = 0; i < position; i++) {
        offset += getBytesSize(dataBlockContent[i]);
    }
    dataBlock.seekp(offset);
    writeDataUnit(dataBlock, dataUnit);
    if(inclusively) {
        i++;
    }
    for(uint j = i; j < dataBlockContent.size(); j++) {
        writeDataUnit(dataBlock, dataBlockContent[j]);
    }
    return dataBlock.good();
}

void SparseIndexFile::clearFile(uint index) {
    uint fileSize = getFileSize(dataBlocks[index]);
    if(fileSize == 0) {
        return;
    }
    std::string dataBlockPath = dataBlockName + std::to_string(index).append(BIN_EXTENSION);
    std::filesystem::resize_file(dataBlockPath.c_str(), 0);
}

void SparseIndexFile::rebuildMainArea() {
    increaseBlocks();
    Table<uint, uint> adjustedIndexArea = rebuildIndexArea();
    std::fstream *dataBlock{nullptr};
    bool exceededThreshold{true};
    uint dataBlockWriteIndex{0}, dataBlockReadIndex{0}, dataBlockContentIndex{0};
    std::streamoff initialFileSize{0}, updatedFileSize{0};
    while(dataBlockReadIndex < adjustedIndexArea.size()) {
        if(exceededThreshold) {
            dataBlock = &dataBlocks[adjustedIndexArea[dataBlockWriteIndex].second];
            initialFileSize = getFileSize(*dataBlock);
            updatedFileSize = 0;
            dataBlock->seekp(0);
            exceededThreshold = false;
        }
        Table<uint, std::string> dataBlockContent = getDataBlockContent(adjustedIndexArea[dataBlockReadIndex].second);
        if(!dataBlockContent.empty()) {
            uint j = dataBlockContentIndex;
            while(j < dataBlockContent.size()
                  && !exceededThreshold) {
                uint threshold = dataBlockWriteIndex == (adjustedIndexArea.size() - 1) ? INFINITE_THRESHOLD
                        : adjustedIndexArea[dataBlockWriteIndex + 1].first;
                if(dataBlockContent[j].first >= threshold) {
                    exceededThreshold = true;
                    tryTruncate(adjustedIndexArea[dataBlockWriteIndex].second, initialFileSize,
                                updatedFileSize);
                    dataBlockWriteIndex++;
                    dataBlockContentIndex = j;
                } else {
                    writeDataUnit(*dataBlock, dataBlockContent[j]);
                    updatedFileSize += getBytesSize(dataBlockContent[j]);
                }
                ++j;
            }
        }
        if(!exceededThreshold) {
            dataBlockReadIndex++;
            dataBlockContentIndex = 0;
        }
    }
    tryTruncate(adjustedIndexArea[dataBlockWriteIndex].second, initialFileSize, updatedFileSize);
    for(uint j = dataBlockWriteIndex + 1; j < adjustedIndexArea.size(); j++) {
        clearFile(adjustedIndexArea[j].second);
    }
}

void SparseIndexFile::tryTruncate(uint blockIndex, std::streamoff initialSize, std::streamoff updatedSize) {
    if(updatedSize < initialSize) {
        std::string dataFilePath = dataBlockName + std::to_string(blockIndex).append(BIN_EXTENSION);
        std::filesystem::resize_file(dataFilePath.c_str(), updatedSize);
    }
}

std::streamoff SparseIndexFile::getBytesSize(const TableRow<uint, std::string> &dataUnit) {
    return static_cast<std::streamoff>(2 * sizeof(uint) + dataUnit.second.size());
}

std::pair<int, bool> SparseIndexFile::searchDataBlock(const Table<uint, std::string> &dataBlock, uint key) {
    int left{0}, right{static_cast<int>(dataBlock.size() - 1)};
    int middle = (left + right) / 2 + -1 * (right < 0);
    int goalIndex{INVALID_INDEX};
    while(goalIndex == INVALID_INDEX
        && left <= right) {
        if(dataBlock[left].first == key) {
            goalIndex = left;
        }
        else if(dataBlock[right].first == key) {
            goalIndex = right;
        }
        else if(dataBlock[middle].first == key) {
            goalIndex = middle;
        }
        else {
            if(key < dataBlock[middle].first) {
                right = middle - 1;
                middle = (left + right) / 2 + -1 * (right < 0);
            } else {
                left = middle + 1;
                middle = (left + right) / 2;
            }
        }
    }
    if(goalIndex == INVALID_INDEX) {
        return std::make_pair(middle + 1, false);
    }
    return std::make_pair(goalIndex, true);
}

std::pair<int, bool>
SparseIndexFile::findValue(uint key, uint *dataBlockIndex, Table<uint, std::string> *dataBlockContent) {
    uint blockIndex = retrieveDataBlockIndex(key);
    Table<uint, std::string> blockContent = getDataBlockContent(blockIndex);
    if(dataBlockIndex) {
        *dataBlockIndex = blockIndex;
    }
    if(dataBlockContent) {
        *dataBlockContent = blockContent;
    }
    return searchDataBlock(blockContent, key);
}

void SparseIndexFile::saveProperties() {
    if(properties) {
        properties.seekp(0);
    } else {
        openFile(properties, indexFilePropertiesPath.c_str(), CREATE_MODE);
    }
    properties.write(reinterpret_cast<const char *>(&blocksCount), sizeof(uint));
    properties.write(reinterpret_cast<const char *>(&blocksSize), sizeof(uint));
    properties.write(reinterpret_cast<const char *>(&increaseFactor), sizeof(uint));
}

void SparseIndexFile::tryLoadProperties() {
    properties.open(indexFilePropertiesPath.c_str(), OPEN_MODE);
    if(!properties) {
        return;
    }
    properties.seekg(0);
    properties.read(reinterpret_cast<char *>(&blocksCount), sizeof(uint));
    properties.read(reinterpret_cast<char *>(&blocksSize), sizeof(uint));
    properties.read(reinterpret_cast<char *>(&increaseFactor), sizeof(uint));
}

bool SparseIndexFile::insert(const TableRow<uint, std::string> &dataUnit) {
    uint dataBlockIndex;
    Table<uint, std::string> dataBlockContent;
    auto searchData = findValue(dataUnit.first, &dataBlockIndex, &dataBlockContent);
    uint dataBlockContentSize = dataBlockContent.size();
    if(searchData.second) {
        return false;
    }
    bool insertionSucceed = insertDataUnit(dataUnit, dataBlockIndex, dataBlockContent, searchData.first, false);
    if(!insertionSucceed) {
        return false;
    }
    dataBlockContentSize++;
    if(dataBlockContentSize == blocksSize) {
        rebuildMainArea();
    }
    return true;
}

bool SparseIndexFile::find(uint key, TableRow<uint, std::string> &dataUnit) {
    Table<uint, std::string> dataBlockContent;
    auto searchData = findValue(key, nullptr, &dataBlockContent);
    if(searchData.second) {
        dataUnit = dataBlockContent[searchData.first];
        return true;
    }
    return false;
}

bool SparseIndexFile::edit(const TableRow<uint, std::string> &dataUnit) {
    uint dataBlockIndex;
    Table<uint, std::string> dataBlockContent;
    auto searchData = findValue(dataUnit.first, &dataBlockIndex, &dataBlockContent);
    if(!searchData.second) {
        return false;
    }
    return insertDataUnit(dataUnit, dataBlockIndex, dataBlockContent, searchData.first, true);
}

bool SparseIndexFile::erase(uint key) {
    uint dataBlockIndex;
    Table<uint, std::string> dataBlockContent;
    auto searchData = findValue(key, &dataBlockIndex, &dataBlockContent);
    if(!searchData.second) {
        return false;
    }
    std::streamoff offset{0};
    uint i;
    for(i = 0; i < searchData.first; i++) {
        offset += getBytesSize(dataBlockContent[i]);
    }
    std::streamoff fileSize{offset};
    std::fstream &dataBlock = dataBlocks[dataBlockIndex];
    dataBlock.seekp(offset);
    for(uint j = i + 1; j < dataBlockContent.size(); j++) {
        writeDataUnit(dataBlock, dataBlockContent[j]);
        fileSize += getBytesSize(dataBlockContent[j]);
    }
    std::string dataFilePath = dataBlockName + std::to_string(dataBlockIndex).append(BIN_EXTENSION);
    std::filesystem::resize_file(dataFilePath.c_str(), fileSize);
    return true;
}

void SparseIndexFile::readIndexArea() {
    indexArea.seekg(0);
    uint currentIndexLimit, blockIndex;
    for(uint i = 0; i < blocksCount; i++) {
        indexArea.read(reinterpret_cast<char *>(&currentIndexLimit), sizeof(uint));
        std::cout<< currentIndexLimit<<' ';
        indexArea.read(reinterpret_cast<char *>(&blockIndex), sizeof(uint));
        std::cout<< blockIndex<<'\n';
    }
}

void SparseIndexFile::shut() {
    indexArea.close();
    for(auto &dataBlock : dataBlocks) {
        dataBlock.close();
    }
    saveProperties();
    properties.close();
}

void SparseIndexFile::foo() {
    dataBlocks[0].seekg(0);
    auto dataBlock = getDataBlockContent(9);
    for(const auto &content : dataBlock) {
        std::cout<<content.first<<" "<<content.second<<'\n';
    }
}
