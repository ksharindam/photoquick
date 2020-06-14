#include "dialogs.h"
#include "common.h"
#include <QDialogButtonBox>
#include <QGridLayout>
#include <cmath>

// Dialog to set JPG image quality for saving
QualityDialog:: QualityDialog(QWidget *parent, QImage &img) : QDialog(parent), image(img)
{
    setWindowTitle("Set Compression");
	timer = new QTimer(this);
	timer->setSingleShot(true);
	timer->setInterval(800);
    QLabel *qualityLabel = new QLabel("Compression Level :", this);
    qualitySpin = new QSpinBox(this);
    qualitySpin->setAlignment(Qt::AlignHCenter);
    qualitySpin->setSuffix(" %");
    qualitySpin->setRange(10,100);
    qualitySpin->setValue(75);
    QCheckBox *showSizeCheck = new QCheckBox("Show File Size", this);
    sizeLabel = new QLabel("Size : Calculating...", this);
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,
                                                    Qt::Horizontal, this);
    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(qualityLabel);
    layout->addWidget(qualitySpin);
    layout->addWidget(showSizeCheck);
    layout->addWidget(sizeLabel);
    layout->addWidget(btnBox);
    sizeLabel->hide();
    connect(showSizeCheck, SIGNAL(clicked(bool)), this, SLOT(toggleCheckSize(bool)));
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void
QualityDialog:: toggleCheckSize(bool checked)
{
    if (checked) {
        sizeLabel->show();
        connect(qualitySpin, SIGNAL(valueChanged(int)), timer, SLOT(start()));
        connect(timer, SIGNAL(timeout()), this, SLOT(checkFileSize()));
        checkFileSize();
    }
    else {
        timer->stop();
        sizeLabel->hide();
        disconnect(qualitySpin, SIGNAL(valueChanged(int)), timer, SLOT(start()));
        disconnect(timer, SIGNAL(timeout()), this, SLOT(checkFileSize()));
    }
}

void
QualityDialog:: checkFileSize()
{
	int filesize = getJpgFileSize(image, qualitySpin->value());
	QString text = "Size : %1 KB";
	sizeLabel->setText(text.arg(QString::number(filesize/1024.0, 'f', 1)));
}

// dialog to choose paper size
PaperSizeDialog:: PaperSizeDialog(QWidget *parent, bool landscapeMode) : QDialog(parent)
{
    this->resize(250, 120);
    this->setWindowTitle("Paper Size");
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    QLabel *label = new QLabel("Select Paper Size :", this);
    combo = new QComboBox(this);
    combo->addItem("Automatic");
    combo->addItem("A4");
    combo->addItem("A5");
    landscape = new QCheckBox("Landscape", this);
    landscape->setChecked(landscapeMode);
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal, this);
    vLayout->addWidget(label);
    vLayout->addWidget(combo);
    vLayout->addWidget(landscape);
    vLayout->addWidget(btnBox);
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));
}


BimodThreshDialog:: BimodThreshDialog(QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle("Bimodal Threshold");
    this->resize(320, 158);

    QGridLayout *gridLayout = new QGridLayout(this);

    QLabel *label = new QLabel("Count :", this);
    gridLayout->addWidget(label, 0, 0, 1, 1);

    countSpin = new QSpinBox(this);
    countSpin->setAlignment(Qt::AlignCenter);
    countSpin->setRange(2, 255);
    gridLayout->addWidget(countSpin, 0, 1, 1, 1);

    QLabel *label_2 = new QLabel("Delta :", this);
    gridLayout->addWidget(label_2, 1, 0, 1, 1);

    deltaSpin = new QSpinBox(this);
    deltaSpin->setAlignment(Qt::AlignCenter);
    deltaSpin->setRange(0,255);
    deltaSpin->setValue(0);
    gridLayout->addWidget(deltaSpin, 1, 1, 1, 1);

    grayBtn = new QCheckBox("Gray", this);
    gridLayout->addWidget(grayBtn, 2, 0, 1, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    gridLayout->addWidget(buttonBox, 3, 0, 1, 2);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

