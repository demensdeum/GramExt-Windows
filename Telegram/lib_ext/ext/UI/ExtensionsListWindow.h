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

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QLabel* createClickableImageLabel(const QString& imageUrl, const QString& linkUrl);
    void populateExtensionsList();
    void showAddExtensionDialog();
};
