#include <QtConcurrent/QtConcurrent>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

#include "driverdetailsdialog.h"
#include "driverinfo.h"
#include "systeminfo.h"

DriverDetailsDialog::DriverDetailsDialog(QWidget *parent) : QDialog(parent) {
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
    connect(listDriverFiles_,
            &QListWidget::currentRowChanged,
            this,
            &DriverDetailsDialog::onFileSelectionChanged);
    mainLayout->addWidget(listDriverFiles_);

    mainLayout->addSpacing(10);

    // File details grid
    auto *detailsLayout = new QGridLayout();
    detailsLayout->setColumnStretch(1, 1);

    auto row = 0;

    labelProvider_ = new QLabel(tr("Provider:"), this);
    labelProviderValue_ = new QLabel(this);
    labelProviderValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    detailsLayout->addWidget(labelProvider_, row, 0);
    detailsLayout->addWidget(labelProviderValue_, row, 1);
    row++;

    labelFileVersion_ = new QLabel(tr("File version:"), this);
    labelFileVersionValue_ = new QLabel(this);
    labelFileVersionValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    detailsLayout->addWidget(labelFileVersion_, row, 0);
    detailsLayout->addWidget(labelFileVersionValue_, row, 1);
    row++;

    labelCopyright_ = new QLabel(tr("Copyright:"), this);
    labelCopyrightValue_ = new QLabel(this);
    labelCopyrightValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelCopyrightValue_->setWordWrap(true);
    detailsLayout->addWidget(labelCopyright_, row, 0);
    detailsLayout->addWidget(labelCopyrightValue_, row, 1);
    row++;

    labelDigitalSigner_ = new QLabel(tr("Digital signer:"), this);
    labelDigitalSignerValue_ = new QLabel(this);
    labelDigitalSignerValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    detailsLayout->addWidget(labelDigitalSigner_, row, 0);
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

void DriverDetailsDialog::setDriverName(const QString &driverName) {
    driverName_ = driverName;
    labelDeviceName_->setText(tr("Driver: %1").arg(driverName));
    populateDriverFiles();
}

void DriverDetailsDialog::setCategoryIcon(const QIcon &icon) {
    categoryIcon_ = icon;
    labelIcon_->setPixmap(icon.pixmap(32, 32));
}

void DriverDetailsDialog::populateDriverFiles() {
    listDriverFiles_->clear();

    if (driverName_.isEmpty()) {
        return;
    }

    listDriverFiles_->addItem(tr("Searching..."));

    if (fileSearchWatcher_) {
        fileSearchWatcher_->cancel();
        fileSearchWatcher_->deleteLater();
    }

    fileSearchWatcher_ = new QFutureWatcher<DriverSearchResult>(this);
    connect(fileSearchWatcher_,
            &QFutureWatcher<DriverSearchResult>::finished,
            this,
            &DriverDetailsDialog::onDriverFilesFound);

    auto driverName = driverName_;
    fileSearchWatcher_->setFuture(
        QtConcurrent::run([driverName]() { return findDriverFiles(driverName); }));
}

void DriverDetailsDialog::onDriverFilesFound() {
    listDriverFiles_->clear();

    if (!fileSearchWatcher_) {
        return;
    }

    auto result = fileSearchWatcher_->result();
    fileSearchWatcher_->deleteLater();
    fileSearchWatcher_ = nullptr;

    if (result.paths.isEmpty()) {
        if (result.isBuiltin) {
            auto builtinInfo = getBuiltinDriverInfo();
            listDriverFiles_->addItem(builtinInfo.builtinMessage);
        } else {
            listDriverFiles_->addItem(tr("(Driver file not found)"));
        }
    } else {
        for (const auto &path : result.paths) {
            auto *item = new QListWidgetItem(formatDriverPath(path));
            item->setData(Qt::UserRole, path);
            listDriverFiles_->addItem(item);
        }
    }

    if (listDriverFiles_->count() > 0) {
        listDriverFiles_->setCurrentRow(0);
    }
}

void DriverDetailsDialog::onFileSelectionChanged() {
    QListWidgetItem *item = listDriverFiles_->currentItem();
    if (!item) {
        labelProviderValue_->clear();
        labelFileVersionValue_->clear();
        labelCopyrightValue_->clear();
        labelDigitalSignerValue_->clear();
        return;
    }

    auto driverPath = item->data(Qt::UserRole).toString();
    if (driverPath.isEmpty()) {
        // Built-in driver - show platform defaults
        auto builtinInfo = getBuiltinDriverInfo();
        labelProviderValue_->setText(builtinInfo.provider);
        labelFileVersionValue_->setText(builtinInfo.version);
        labelCopyrightValue_->setText(builtinInfo.copyright);
        labelDigitalSignerValue_->setText(builtinInfo.signer);
        return;
    }

    updateFileDetails(driverPath);
}

void DriverDetailsDialog::updateFileDetails(const QString &driverPath) {
    DriverFileDetails details = getDriverFileDetails(driverPath, driverName_);
    labelProviderValue_->setText(details.provider);
    labelFileVersionValue_->setText(details.version);
    labelCopyrightValue_->setText(details.copyright);
    labelDigitalSignerValue_->setText(details.signer);
}
