#pragma once

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <array>
#include "../../utils/utils.h"

class DataBaseView : public QWidget {
private:
    QPushButton *findButton;
    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *editButton;
    QPushButton *randomButton;
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonsLayout;
    QLabel *keyLabel;
    QLabel *valueLabel;
    QLineEdit *keyInputField;
    QLineEdit *valueInputField;
    QVBoxLayout *keyLayout;
    QVBoxLayout *valueLayout;
    QHBoxLayout *inputFieldsLayout;
    QLabel *outputLabel;
    QLabel *output;
    void setUpView();
    void setUpButtons();
    void setUpLineEdits();
    void setUpOutput();
    static void setUpInputField(QLabel *&label, const char *name, QLineEdit *&inputField, QVBoxLayout *&layout);
public:
    explicit DataBaseView(QWidget *parent = nullptr);
    void setOutput(const QString &text);
    NODISCARD QPushButton *getFindButton() const;
    NODISCARD QPushButton *getAddButton() const;
    NODISCARD QPushButton *getRemoveButton() const;
    NODISCARD QPushButton *getEditButton() const;
    NODISCARD QPushButton *getRandomButton() const;
    NODISCARD QLineEdit *getKeyInput() const;
    NODISCARD QLineEdit *getValueInput() const;
};