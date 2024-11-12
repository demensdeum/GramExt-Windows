#include "ExtensionsListWindow.h"
#include <QDesktopServices>
#include <QUrl>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QInputDialog>
#include <QString>
#include <QMessageBox>
#include <ext/Extension/Extension.h>
#include <ext/Controller/Controller.h>

QListWidget* listWidget;

ExtensionsListWindow::ExtensionsListWindow(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Extensions List");
    setMinimumWidth(720);

    listWidget = new QListWidget(this);
    QPushButton* addButton = new QPushButton("Add Extension", this);
    connect(addButton, &QPushButton::clicked, this, &ExtensionsListWindow::showAddExtensionDialog);

    populateExtensionsList();

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(listWidget);
    mainLayout->addWidget(addButton);
    setLayout(mainLayout);
}

void ExtensionsListWindow::populateExtensionsList() {
    listWidget->clear();
    for (const auto& extension : GramExt::Controller::getExtensions()) {
        QString title = QString::fromStdString(extension.title);
        QString info = QString::fromStdString(extension.info);
        QString rootUrl = QString::fromStdString(extension.rootUrl);

        QWidget* itemWidget = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(itemWidget);

        QLabel* titleLabel = new QLabel("<a href=\"" + rootUrl + "\"><b>" + title + "</b></a>", itemWidget);
        titleLabel->setTextFormat(Qt::RichText);
        titleLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        titleLabel->setOpenExternalLinks(false);

        connect(titleLabel, &QLabel::linkActivated, this, [](const QString& link) {
            QDesktopServices::openUrl(QUrl(link));
            });

        QLabel* infoLabel = new QLabel(info, itemWidget);

        QCheckBox* checkBox = new QCheckBox(itemWidget);
        checkBox->setChecked(false);

        connect(checkBox, &QCheckBox::stateChanged, this, [=](int state) {
            if (state == Qt::Checked) {
                GramExt::Controller::enableExtension(extension);
            }
            else {
                GramExt::Controller::disableExtension(extension);
            }
            });

        layout->addWidget(titleLabel);
        layout->addWidget(infoLabel);
        layout->addWidget(checkBox);
        layout->setAlignment(checkBox, Qt::AlignRight);
        layout->setContentsMargins(5, 5, 5, 5);

        itemWidget->setLayout(layout);

        QListWidgetItem* listItem = new QListWidgetItem(listWidget);
        listItem->setSizeHint(itemWidget->sizeHint());
        listWidget->addItem(listItem);
        listWidget->setItemWidget(listItem, itemWidget);
    }
}

void ExtensionsListWindow::showAddExtensionDialog() {
    bool ok;
    QString extensionURL = QInputDialog::getText(this, "Add Extension", "Enter Extension URL:", QLineEdit::Normal, "", &ok);
    if (ok && !extensionURL.isEmpty()) {
        GramExt::Controller::addExtension(extensionURL.toStdString());
        populateExtensionsList();
    }
}
