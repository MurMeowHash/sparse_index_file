#include "Error.h"
#include <iostream>
#include <QApplication>

void Error::fallWithError(const char *msg) {
    std::cerr << formError(msg);
    QApplication::exit(1);
}

const char *Error::formError(const char *msg) {
    return (std::string{"ERROR::"} + msg).c_str();
}