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
#include <curl/curl.h>

QListWidget *listWidget;
QWidget *container;
QLabel *label;
QPushButton *button;
QImage image;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    QByteArray* imageData = static_cast<QByteArray*>(userp);
    imageData->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

void ExtensionsListWindow::resizeEvent(QResizeEvent* event) {
    QDialog::resizeEvent(event);

    int newWidth = this->width();
    int newHeight = static_cast<int>(newWidth * 0.558804831);
    container->setFixedSize(newWidth, newHeight);

    if (!image.isNull()) {
        QPixmap scaledPixmap = QPixmap::fromImage(image).scaled(container->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        label->setPixmap(scaledPixmap);
        label->setGeometry(0, 0, container->width(), container->height());
    }

    label->setGeometry(0, 0, container->width(), container->height());
    button->setGeometry(0, 0, container->width(), container->height());
}

QByteArray downloadImage(const std::string& url) {
    CURL* curl;
    CURLcode res;
    QByteArray imageData;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &imageData);

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);    // Follow redirects
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);          // Prevent signals on timeout

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            qWarning("Failed to download image: %s", curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    else {
        qWarning("Failed to initialize CURL");
    }
    return imageData;
}

QWidget* ExtensionsListWindow::createClickableImageLabel(const QString& imageUrl, const QString& linkUrl) {
    QByteArray imageData = downloadImage(imageUrl.toStdString());

    if (!image.loadFromData(imageData)) {
        qWarning("Failed to load image from data");
        return nullptr;
    }

    container = new QWidget(this);
    container->setFixedSize(this->width(), this->width() * 0.558804831);

    label = new QLabel(container);
    label->setPixmap(QPixmap::fromImage(image).scaled(container->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    label->setGeometry(0, 0, container->width(), container->height());

    button = new QPushButton(container);
    button->setFlat(true);
    button->setStyleSheet("background-color: transparent;");
    button->setGeometry(0, 0, container->width(), container->height());

    connect(button, &QPushButton::clicked, this, [linkUrl]() {
        QDesktopServices::openUrl(QUrl(linkUrl));
    });

    return container;
}

ExtensionsListWindow::ExtensionsListWindow(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Extensions List");
    setMinimumWidth(720);

    listWidget = new QListWidget(this);

    createClickableImageLabel("https://demensdeum.com/logo/demens1.png", "https://demensdeum.com");

    if (!container) {
        return;
    }

    QPushButton* addButton = new QPushButton("Add Extension", this);
    connect(addButton, &QPushButton::clicked, this, &ExtensionsListWindow::showAddExtensionDialog);

    populateExtensionsList();

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(container);
    mainLayout->addWidget(listWidget);
    mainLayout->addWidget(addButton);
    setLayout(mainLayout);
}

void ExtensionsListWindow::populateExtensionsList() {
    listWidget->clear();
    std::set<GramExt::Extension> enabledExtensions = GramExt::Controller::getEnabledExtensions();

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
}

void ExtensionsListWindow::showAddExtensionDialog() {
    bool ok;
    QString extensionURL = QInputDialog::getText(this, "Add Extension", "Enter Extension URL:", QLineEdit::Normal, "", &ok);
    if (ok && !extensionURL.isEmpty()) {
        GramExt::Controller::addExtension(extensionURL.toStdString());
        populateExtensionsList();
    }
}
