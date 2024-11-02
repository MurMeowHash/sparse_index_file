#pragma once

#include "../utils/utils.h"
#include <fstream>
#include <vector>

template <typename T1, typename T2> using TableRow = std::pair<T1, T2>;
template<typename T1, typename T2> using Table = std::vector<TableRow<T1, T2>>;

class SparseIndexFile {
private:
    static constexpr uint INITIAL_BLOCKS_COUNT{10};
    static constexpr uint INITIAL_BLOCKS_SIZE{100};
    static constexpr uint INITIAL_INCREASE_FACTOR{INITIAL_BLOCKS_SIZE};
    static constexpr std::string INDEX_FILES_DIRECTORY{"../index_files/"};
    static constexpr std::string DATA_BLOCK_NAME{"data_block"};
    static constexpr std::string BIN_EXTENSION{".dat"};
    static constexpr std::string INDEX_AREA_NAME{"index_area"};
    static constexpr std::string INDEX_FILE_PROPERTIES_NAME{"properties"};
    static constexpr std::ios::openmode CREATE_MODE{std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc};
    static constexpr std::ios::openmode OPEN_MODE{std::ios::binary | std::ios::in | std::ios::out};
    static constexpr int INVALID_INDEX{-1};
    static const uint INFINITE_THRESHOLD;
    std::string indexFilePropertiesPath;
    std::string indexAreaPath;
    std::string dataBlockName;
    uint blocksCount;
    uint blocksSize;
    uint increaseFactor;
    std::fstream indexArea;
    std::vector<std::fstream> dataBlocks;
    std::fstream properties;
    void openIndexArea();
    void openDataBlocks();
    Table<uint, uint> rebuildIndexArea();
    NODISCARD Table<uint, uint> getIndexArea();
    NODISCARD uint retrieveDataBlockIndex(uint key);
    NODISCARD std::pair<int, bool> findValue(uint key, uint *dataBlockIndex,
                                             Table<uint, std::string> *dataBlockContent);
    NODISCARD Table<uint, std::string> getDataBlockContent(uint dataBlockIndex);
    bool insertDataUnit(const TableRow<uint, std::string> &dataUnit, uint dataBlockIndex,
                        const Table<uint, std::string> &dataBlockContent, uint position, bool inclusively);
    inline void increaseBlocks() {
        blocksSize += increaseFactor;
    }
    void clearFile(uint index);
    void rebuildMainArea();
    void tryTruncate(uint blockIndex, std::streamoff initialSize, std::streamoff updatedSize);
    void saveProperties();
    void tryLoadProperties();
    NODISCARD static std::streamoff getBytesSize(const TableRow<uint, std::string> &dataUnit);
    NODISCARD static TableRow<uint, std::string> readDataUnit(std::fstream &dataBlock);
    static void writeDataUnit(std::fstream &dataBlock, const TableRow<uint, std::string> &dataUnit);
    NODISCARD static std::pair<int, bool> searchDataBlock(const Table<uint, std::string> &dataBlock, uint key);
public:
    explicit SparseIndexFile(uint blocksCount = INITIAL_BLOCKS_COUNT, uint blocksSize = INITIAL_BLOCKS_SIZE,
                             uint increaseFactor = INITIAL_INCREASE_FACTOR);
    bool insert(const TableRow<uint, std::string> &dataUnit);
    NODISCARD bool find(uint key, TableRow<uint, std::string> &dataUnit);
    bool edit(const TableRow<uint, std::string> &dataUnit);
    bool erase(uint key);
    UNUSED void readIndexArea();
    void shut();
    void foo();
};