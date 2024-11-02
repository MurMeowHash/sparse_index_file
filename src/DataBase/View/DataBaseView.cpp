#include "DataBaseView.h"

DataBaseView::DataBaseView(QWidget *parent) : QWidget(parent) {
    setUpView();
}

void DataBaseView::setUpView() {
    setUpButtons();
    setUpLineEdits();
    setUpOutput();
    mainLayout = new QVBoxLayout{this};
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addLayout(inputFieldsLayout);
    mainLayout->addWidget(outputLabel);
    mainLayout->addWidget(output);
    mainLayout->setAlignment(Qt::AlignTop);
}

void DataBaseView::setUpButtons() {
    findButton = new QPushButton{"find"};
    addButton = new QPushButton{"add"};
    removeButton = new QPushButton{"remove"};
    editButton = new QPushButton{"edit"};
    randomButton = new QPushButton{"random fill"};
    buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(findButton);
    buttonsLayout->addWidget(addButton);
    buttonsLayout->addWidget(removeButton);
    buttonsLayout->addWidget(editButton);
    buttonsLayout->addWidget(randomButton);
}

void DataBaseView::setUpLineEdits() {
    setUpInputField(keyLabel, "Key", keyInputField, keyLayout);
    setUpInputField(valueLabel, "Value", valueInputField, valueLayout);
    inputFieldsLayout = new QHBoxLayout;
    inputFieldsLayout->addLayout(keyLayout);
    inputFieldsLayout->addLayout(valueLayout);
}

void DataBaseView::setUpInputField(QLabel *&label, const char *name, QLineEdit *&inputField, QVBoxLayout *&layout) {
    label = new QLabel{name};
    inputField = new QLineEdit;
    layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(inputField);
}

void DataBaseView::setUpOutput() {
    outputLabel = new QLabel{"Output"};
    output = new QLabel;
}

void DataBaseView::setOutput(const QString &text) {
    output->setText(text);
}

QPushButton *DataBaseView::getFindButton() const {
    return findButton;
}

QPushButton *DataBaseView::getAddButton() const {
    return addButton;
}

QPushButton *DataBaseView::getRemoveButton() const {
    return removeButton;
}

QPushButton *DataBaseView::getEditButton() const {
    return editButton;
}

QPushButton *DataBaseView::getRandomButton() const {
    return randomButton;
}

QLineEdit *DataBaseView::getKeyInput() const {
    return keyInputField;
}

QLineEdit *DataBaseView::getValueInput() const {
    return valueInputField;
}
