#include "ExtensionsListWindow.h"
#include <ext/Extension/Extension.h>

ExtensionsListWindow::ExtensionsListWindow(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Extensions List");

    setMinimumWidth(640);

    QListWidget* listWidget = new QListWidget(this);

    for (const auto& extension : GramExt::Controller::getExtensions()) {
        QString info = QString::fromStdString(extension.info);

        QWidget* itemWidget = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(itemWidget);

        std::string rootUrl = extension.rootUrl;
        std::string title = extension.title;
        std::string label = "<a href=\"" + rootUrl + "\"><b>" + title + "</b></a>";

        QLabel* titleLabel = new QLabel(QString::fromStdString(label), itemWidget);
        titleLabel->setTextFormat(Qt::RichText);
        titleLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        titleLabel->setOpenExternalLinks(true);

        QLabel* infoLabel = new QLabel(info, itemWidget);

        std::set<GramExt::Extension> enabledExtensions = GramExt::Controller::getEnabledExtensions();

        QCheckBox* checkBox = new QCheckBox(itemWidget);
        checkBox->setChecked(enabledExtensions.contains(extension));

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

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(listWidget);
    setLayout(mainLayout);
}
