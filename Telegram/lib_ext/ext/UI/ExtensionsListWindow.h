#pragma once

#include <QDialog>
#include <QListWidget>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QString>
#include <QWidget>

class ExtensionsListWindow : public QDialog {
    Q_OBJECT

public:
    explicit ExtensionsListWindow(QWidget* parent = nullptr);

private:
    void populateExtensionsList();
    void showAddExtensionDialog();
};
