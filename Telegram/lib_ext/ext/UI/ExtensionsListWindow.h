#pragma once

#include <QDialog>
#include <QListWidget>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QString>
#include <QWidget>
#include <ext/Controller/Controller.h>

class ExtensionsListWindow : public QDialog {
    Q_OBJECT

public:
    explicit ExtensionsListWindow(QWidget* parent = nullptr);
};
