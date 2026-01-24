#include "viewerdriverdetailsdialog.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

ViewerDriverDetailsDialog::ViewerDriverDetailsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Driver File Details"));
    setMinimumSize(450, 350);

    auto *mainLayout = new QVBoxLayout(this);

    // Top section with icon and driver name
    auto *topLayout = new QHBoxLayout();
    labelIcon_ = new QLabel(this);
    labelIcon_->setFixedSize(32, 32);
    topLayout->addWidget(labelIcon_);

    topLayout->addSpacing(10);

    labelDeviceName_ = new QLabel(this);
    labelDeviceName_->setWordWrap(true);
    topLayout->addWidget(labelDeviceName_, 1);

    mainLayout->addLayout(topLayout);
    mainLayout->addSpacing(10);

    // Driver files section
    auto *filesLabel = new QLabel(tr("Driver files:"), this);
    mainLayout->addWidget(filesLabel);

    listDriverFiles_ = new QListWidget(this);
    listDriverFiles_->setMinimumHeight(100);
    mainLayout->addWidget(listDriverFiles_);

    mainLayout->addSpacing(10);

    // File details grid
    auto *detailsLayout = new QGridLayout();
    detailsLayout->setColumnStretch(1, 1);

    int row = 0;

    detailsLayout->addWidget(new QLabel(tr("Provider:"), this), row, 0);
    labelProviderValue_ = new QLabel(this);
    labelProviderValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    detailsLayout->addWidget(labelProviderValue_, row, 1);
    row++;

    detailsLayout->addWidget(new QLabel(tr("File version:"), this), row, 0);
    labelFileVersionValue_ = new QLabel(this);
    labelFileVersionValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    detailsLayout->addWidget(labelFileVersionValue_, row, 1);
    row++;

    detailsLayout->addWidget(new QLabel(tr("Copyright:"), this), row, 0);
    labelCopyrightValue_ = new QLabel(this);
    labelCopyrightValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelCopyrightValue_->setWordWrap(true);
    detailsLayout->addWidget(labelCopyrightValue_, row, 1);
    row++;

    detailsLayout->addWidget(new QLabel(tr("Digital signer:"), this), row, 0);
    labelDigitalSignerValue_ = new QLabel(this);
    labelDigitalSignerValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    detailsLayout->addWidget(labelDigitalSignerValue_, row, 1);

    mainLayout->addLayout(detailsLayout);

    // Spacer
    mainLayout->addStretch();

    // OK button
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    auto *okButton = new QPushButton(tr("OK"), this);
    okButton->setDefault(true);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(okButton);
    mainLayout->addLayout(buttonLayout);
}

void ViewerDriverDetailsDialog::setDriverName(const QString &driverName) {
    driverName_ = driverName;
    labelDeviceName_->setText(tr("Driver: %1").arg(driverName));
}

void ViewerDriverDetailsDialog::setDriverInfo(const QJsonObject &driverInfo) {
    driverInfo_ = driverInfo;
    populateDetails();
}

void ViewerDriverDetailsDialog::setCategoryIcon(const QIcon &icon) {
    categoryIcon_ = icon;
    labelIcon_->setPixmap(icon.pixmap(32, 32));
}

void ViewerDriverDetailsDialog::populateDetails() {
    listDriverFiles_->clear();

    // Get driver files from exported data
    QJsonArray files = driverInfo_[QStringLiteral("files")].toArray();
    QString filename = driverInfo_[QStringLiteral("filename")].toString();

    if (!filename.isEmpty()) {
        listDriverFiles_->addItem(filename);
    }

    for (const QJsonValue val : files) {
        QString file = val.toString();
        if (!file.isEmpty() && file != filename) {
            listDriverFiles_->addItem(file);
        }
    }

    // If no files found, show built-in message
    if (listDriverFiles_->count() == 0) {
        if (driverInfo_[QStringLiteral("builtin")].toBool()) {
            listDriverFiles_->addItem(tr("(Built-in kernel module)"));
        } else {
            listDriverFiles_->addItem(tr("(No driver files recorded)"));
        }
    }

    // Provider
    QString provider = driverInfo_[QStringLiteral("provider")].toString();
    if (provider.isEmpty()) {
        provider = driverInfo_[QStringLiteral("author")].toString();
    }
    if (provider.isEmpty()) {
        provider = QStringLiteral("Linux Foundation");
    }
    labelProviderValue_->setText(provider);

    // Version
    QString version = driverInfo_[QStringLiteral("version")].toString();
    if (version.isEmpty()) {
        version = driverInfo_[QStringLiteral("kernelVersion")].toString();
    }
    if (version.isEmpty()) {
        version = tr("N/A");
    }
    labelFileVersionValue_->setText(version);

    // Copyright/License
    QString license = driverInfo_[QStringLiteral("license")].toString();
    if (license.isEmpty()) {
        license = tr("N/A");
    }
    labelCopyrightValue_->setText(license);

    // Digital signer
    QString signer = driverInfo_[QStringLiteral("signer")].toString();
    if (signer.isEmpty()) {
        signer = provider;
    }
    labelDigitalSignerValue_->setText(signer);
}
