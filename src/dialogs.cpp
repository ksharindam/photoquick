// this file is part of photoquick program which is GPLv3 licensed
#include "dialogs.h"
#include "common.h"
#include "filters.h"
#include <QDialogButtonBox>
#include <QGridLayout>
#include <cmath>

// ------------ Dialog to set JPG image quality for saving ------------

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


// -----------------  dialog to choose paper size ----------------------

PaperSizeDialog:: PaperSizeDialog(QWidget *parent, bool landscapeMode) : QDialog(parent)
{
    this->resize(250, 120);
    this->setWindowTitle("Paper Size");
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    QLabel *label = new QLabel("Select Paper Size :", this);
    combo = new QComboBox(this);
    QStringList items = { "Automatic", "A4", "A5", "100 dpi", "300 dpi", "Other dpi" };
    combo->addItems(items);
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


// --------------- dialog to choose border width and size --------------

ExpandBorderDialog:: ExpandBorderDialog(QWidget *parent, int border_w) : QDialog(parent)
{
    this->resize(250, 120);
    this->setWindowTitle("Expand Image Border");
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    QLabel *label = new QLabel("Expand each side by :", this);
    widthSpin = new QSpinBox(this);
    widthSpin->setAlignment(Qt::AlignHCenter);
    widthSpin->setSuffix(" px");
    widthSpin->setRange(1, border_w*5);
    widthSpin->setValue(border_w);
    QLabel *label2 = new QLabel("Set Border Type :", this);
    combo = new QComboBox(this);
    QStringList items = {"Clone Edges", "White Color", "Black Color", "Other Color"};
    combo->addItems(items);
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal, this);
    vLayout->addWidget(label);
    vLayout->addWidget(widthSpin);
    vLayout->addWidget(label2);
    vLayout->addWidget(combo);
    vLayout->addWidget(btnBox);
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));
}


//------------------ PreviewDialog for Filters ------------------

PreviewDialog:: PreviewDialog(QLabel *canvas, QImage img, float scale) : QDialog(canvas)
{
    this->image = img;
    this->scale = scale;
	timer = new QTimer(this);
	timer->setSingleShot(true);
	timer->setInterval(800);
    connect(timer, SIGNAL(timeout()), this, SLOT(run()));
    connect(this, SIGNAL(previewRequested(const QPixmap&)), canvas, SLOT(setPixmap(const QPixmap&)));
}

void
PreviewDialog:: onValueChange()
{
    timer->start();
}

void
PreviewDialog:: preview(QImage img)
{
    QPixmap pm = QPixmap::fromImage(img);
    if (scale != 1.0) {
        Qt::TransformationMode mode = floorf(scale) == ceilf(scale)? // integer scale
                                    Qt::FastTransformation : Qt::SmoothTransformation;
        pm = pm.scaledToHeight(scale*pm.height(), mode);
    }
    previewRequested(pm);
}


// ----------- Preview Dialog for Lens Distortion Correction --------- //

LensDialog:: LensDialog(QLabel *canvas, QImage img, float scale) : PreviewDialog(canvas,img,scale)
{
    setWindowTitle("Lens Distortion");
    QLabel *label0 = new QLabel("Main :", this);
    QLabel *label1 = new QLabel("Edge :", this);
    QLabel *label2 = new QLabel("Zoom :", this);
    mainSpin = new QDoubleSpinBox(this);
    mainSpin->setDecimals(1);
    mainSpin->setRange(-100, 100);
    mainSpin->setValue(main);
    edgeSpin = new QDoubleSpinBox(this);
    edgeSpin->setDecimals(1);
    edgeSpin->setRange(-100, 100);
    edgeSpin->setValue(edge);
    zoomSpin = new QDoubleSpinBox(this);
    zoomSpin->setDecimals(1);
    zoomSpin->setRange(-100, 100);
    zoomSpin->setValue(zoom);
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok|
                                    QDialogButtonBox::Cancel, Qt::Horizontal, this);
    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(label0, 0,0,1,1);
    layout->addWidget(label1, 1,0,1,1);
    layout->addWidget(label2, 2,0,1,1);
    layout->addWidget(mainSpin, 0,1,1,1);
    layout->addWidget(edgeSpin, 1,1,1,1);
    layout->addWidget(zoomSpin, 2,1,1,1);
    layout->addWidget(btnBox, 3,0,1,2);
    connect(mainSpin, SIGNAL(valueChanged(double)), this, SLOT(onValueChange()));
    connect(edgeSpin, SIGNAL(valueChanged(double)), this, SLOT(onValueChange()));
    connect(zoomSpin, SIGNAL(valueChanged(double)), this, SLOT(onValueChange()));
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));

    onValueChange();
}

void
LensDialog:: run()
{
    main = mainSpin->value();
    edge = edgeSpin->value();
    zoom = zoomSpin->value();

    QImage img = image.copy();
    lensDistortion(img, main, edge, zoom);
    preview(img);
}
