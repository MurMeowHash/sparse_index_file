#include "DataBaseController.h"
#include "../../Error/Error.h"
#include "../../Random/Random.h"
#include <algorithm>

DataBaseController::DataBaseController() {
    mainWindow = new QMainWindow;
    dataBaseView = new DataBaseView{mainWindow};
    mainWindow->setCentralWidget(dataBaseView);
    dataBaseModel = new SparseIndexFile{10, 100, 100};
    setUpConnections();
    mainWindow->show();
}

void DataBaseController::setUpConnections() {
    bool connectionSucceed{true};
    connectionSucceed *= static_cast<bool>(connect(dataBaseView->getFindButton(), &QPushButton::clicked,
                                this, &DataBaseController::findValue));
    connectionSucceed *= static_cast<bool>(connect(dataBaseView->getAddButton(), &QPushButton::clicked,
                                                   this, &DataBaseController::addValue));
    connectionSucceed *= static_cast<bool>(connect(dataBaseView->getRemoveButton(), &QPushButton::clicked,
                                                   this, &DataBaseController::removeValue));
    connectionSucceed *= static_cast<bool>(connect(dataBaseView->getEditButton(), &QPushButton::clicked,
                                                   this, &DataBaseController::editValue));
    connectionSucceed *= static_cast<bool>(connect(dataBaseView->getRandomButton(), &QPushButton::clicked,
                                                   this, &DataBaseController::fillDataBase));
    if(!connectionSucceed) {
        shut();
        Error::fallWithError("FAILED_TO_LOAD_CONNECTIONS");
    }
}

void DataBaseController::shut() {
    delete mainWindow;
    dataBaseModel->shut();
    delete dataBaseModel;
}

bool DataBaseController::retrieveKeyInput(uint &key) {
    QLineEdit *keyField = dataBaseView->getKeyInput();
    const QString &keyInput = keyField->text();
    bool conversionSucceed;
    key = keyInput.toUInt(&conversionSucceed);
    return conversionSucceed;
}

std::string DataBaseController::retrieveValueInput() {
    QLineEdit *valueField = dataBaseView->getValueInput();
    return valueField->text().toStdString();
}

Table<uint, std::string> DataBaseController::genKeyValuePairs(uint keyUpperLimit) {
    Table<uint, std::string> content(keyUpperLimit + 1);
    for(uint i = 0; i < content.size(); i++) {
        content[i] = TableRow<uint, std::string>(i, genRandomString(8));
    }
    return content;
}

void DataBaseController::findValue() {
    uint key;
    bool succeedRetrieve = retrieveKeyInput(key);
    QString output;
    if(!succeedRetrieve) {
        output = "Invalid input";
    } else {
        TableRow<uint, std::string> retrievedData;
        if(dataBaseModel->find(key, retrievedData)) {
            output = retrievedData.second.c_str();
        } else {
            output = "Item with key " + QString::number(key) + " does not exist";
        }
    }
    dataBaseView->setOutput(output);
}

void DataBaseController::addValue() {
    uint key;
    bool succeedRetrieve = retrieveKeyInput(key);
    std::string value = retrieveValueInput();
    QString output;
    if(!succeedRetrieve) {
        output = "Invalid input";
    } else {
        if(dataBaseModel->insert(TableRow<uint, std::string>(key, value))) {
            output = "Key: " + QString::number(key) + " Value: " + QString(value.c_str()) + " was successfully added";
        } else {
            output = "Item with key " + QString::number(key) + " already exists";
        }
    }
    dataBaseView->setOutput(output);
}

void DataBaseController::removeValue() {
    uint key;
    bool succeedRetrieve = retrieveKeyInput(key);
    QString output;
    if(!succeedRetrieve) {
        output = "Invalid input";
    } else {
        if(dataBaseModel->erase(key)) {
            output = "Item with key " + QString::number(key) + " was successfully erased";
        } else {
            output = "Item with key " + QString::number(key) + " does not exist";
        }
    }
    dataBaseView->setOutput(output);
}

void DataBaseController::editValue() {
    uint key;
    bool succeedRetrieve = retrieveKeyInput(key);
    std::string value = retrieveValueInput();
    QString output;
    if(!succeedRetrieve) {
        output = "Invalid input";
    } else {
        if(dataBaseModel->edit(TableRow<uint, std::string>(key, value))) {
            output = "Item with key " + QString::number(key) + " was successfully edited to value " + QString(value.c_str());
        } else {
            output = "Item with key " + QString::number(key) + " does not exist";
        }
    }
    dataBaseView->setOutput(output);
}

DataBaseController::~DataBaseController() {
    shut();
}

void DataBaseController::fillDataBase() {
    auto data = genKeyValuePairs(RANDOM_KEY_UPPER_LIMIT);
    std::shuffle(data.begin(), data.end(), Random::getRandomDevice());
    for(const auto &dataUnit : data) {
        dataBaseModel->insert(dataUnit);
    }
    QString output = "Data base was successfully filled with " + QString::number(data.size()) + " units";
    dataBaseView->setOutput(output);
}