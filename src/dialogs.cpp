#include "dialogs.h"
#include "common.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QBuffer>
#include <QByteArray>
#include <cmath>

// Dialog to set JPG image quality for saving
QualityDialog:: QualityDialog(QWidget *parent, QImage &img) : QDialog(parent), image(img)
{
    setWindowTitle("Set Quality");
	timer = new QTimer(this);
	timer->setSingleShot(true);
	timer->setInterval(800);
    QLabel *qualityLabel = new QLabel("Set Image Quality (%):", this);
    qualitySpin = new QSpinBox(this);
    qualitySpin->setAlignment(Qt::AlignHCenter);
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
	QByteArray bArray;
	QBuffer buffer(&bArray);
	buffer.open(QIODevice::WriteOnly);
	image.save(&buffer, "JPG", qualitySpin->value());
	int filesize = bArray.size();
	bArray.clear();
	buffer.close();
	QString text = "Size : %1 KB";
	sizeLabel->setText(text.arg(QString::number(filesize/1024.0, 'f', 1)));
}


