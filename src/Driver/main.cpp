#include <QApplication>
#include "../DataBase/Controllers/DataBaseController.h"

int main(int argc, char **argv) {
    QApplication app{argc, argv};
    auto dataBaseController = new DataBaseController;
    int execCode = QApplication::exec();
    delete dataBaseController;
    return execCode;
}