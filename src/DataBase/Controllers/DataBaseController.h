#pragma once

#include <QMainWindow>
#include "../View/DataBaseView.h"
#include "../../SparseIndex/SparseIndexFile.h"
#include <vector>

class DataBaseController : public QObject {
    Q_OBJECT
private:
    static constexpr uint RANDOM_KEY_UPPER_LIMIT{9999};
    QMainWindow *mainWindow;
    DataBaseView *dataBaseView;
    SparseIndexFile *dataBaseModel;
    void setUpConnections();
    void shut();
    NODISCARD bool retrieveKeyInput(uint &key);
    NODISCARD std::string retrieveValueInput();
    static Table<uint, std::string> genKeyValuePairs(uint keyUpperLimit);
public:
    explicit DataBaseController();
    void findValue();
    void addValue();
    void removeValue();
    void editValue();
    void fillDataBase();
    ~DataBaseController() override;
};