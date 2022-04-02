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
    this->canvas = canvas;
    this->image = img;
    this->scale = scale;
    timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(800);
    connect(timer, SIGNAL(timeout()), this, SLOT(preview()));
}

void
PreviewDialog:: triggerPreview()
{
    timer->start();
}

void
PreviewDialog:: preview()
{
    QImage img = getResult(image);

    QPixmap pm = QPixmap::fromImage(img);
    if (scale != 1.0) {
        Qt::TransformationMode mode = floorf(scale) == ceilf(scale)? // integer scale
                                    Qt::FastTransformation : Qt::SmoothTransformation;
        pm = pm.scaledToHeight(scale*pm.height(), mode);
    }
    canvas->setPixmap(pm);
}


// ----------- Preview Dialog for Rotate Any Degree --------- //

RotateDialog:: RotateDialog(QLabel *canvas, QImage img, float scale) : PreviewDialog(canvas,img,scale)
{
    setWindowTitle("Rotate by Any Angle");
    QLabel *label0 = new QLabel("Enter Angle :", this);
    angleSpin = new QSpinBox(this);
    angleSpin->setRange(-359, 359);
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                    QDialogButtonBox::Cancel, Qt::Horizontal, this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label0);
    layout->addWidget(angleSpin);
    layout->addWidget(btnBox);

    connect(angleSpin, SIGNAL(valueChanged(int)), this, SLOT(triggerPreview()));
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));
}

QImage
RotateDialog:: getResult(QImage img)
{
    angle = angleSpin->value();

    QTransform transform;
    transform.rotate(angle, Qt::ZAxis);
    return img.transformed(transform, Qt::SmoothTransformation);
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

    connect(mainSpin, SIGNAL(valueChanged(double)), this, SLOT(triggerPreview()));
    connect(edgeSpin, SIGNAL(valueChanged(double)), this, SLOT(triggerPreview()));
    connect(zoomSpin, SIGNAL(valueChanged(double)), this, SLOT(triggerPreview()));
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));

    triggerPreview();
}

QImage
LensDialog:: getResult(QImage img)
{
    main = mainSpin->value();
    edge = edgeSpin->value();
    zoom = zoomSpin->value();

    lensDistortion(img, main, edge, zoom);
    return img;
}


// ----------- Preview Dialog for Threshold Filter --------- //

ThresholdDialog:: ThresholdDialog(QLabel *canvas, QImage img, float scale) : PreviewDialog(canvas,img,scale)
{
    setWindowTitle("Threshold Value");
    QLabel *label0 = new QLabel("Enter threshold Value :", this);
    thresholdSpin = new QSpinBox(this);
    thresholdSpin->setRange(1, 254);
    thresholdSpin->setValue( calcOtsuThresh(img) );
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                    QDialogButtonBox::Cancel, Qt::Horizontal, this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label0);
    layout->addWidget(thresholdSpin);
    layout->addWidget(btnBox);

    connect(thresholdSpin, SIGNAL(valueChanged(int)), this, SLOT(triggerPreview()));
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));

    triggerPreview();
}

QImage
ThresholdDialog:: getResult(QImage img)
{
    thresh = thresholdSpin->value();

    threshold(img, thresh);
    return img;
}


// ----------- Preview Dialog for Gamma Correction Filter --------- //

GammaDialog:: GammaDialog(QLabel *canvas, QImage img, float scale) : PreviewDialog(canvas,img,scale)
{
    setWindowTitle("Apply Gamma");
    QLabel *label0 = new QLabel("Enter the value of Gamma :", this);
    gammaSpin = new QDoubleSpinBox(this);
    gammaSpin->setSingleStep(0.05);
    gammaSpin->setRange(0.1, 10.0);
    gammaSpin->setValue( gamma );
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                    QDialogButtonBox::Cancel, Qt::Horizontal, this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label0);
    layout->addWidget(gammaSpin);
    layout->addWidget(btnBox);

    connect(gammaSpin, SIGNAL(valueChanged(double)), this, SLOT(triggerPreview()));
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));

    triggerPreview();
}

QImage
GammaDialog:: getResult(QImage img)
{
    gamma = gammaSpin->value();

    applyGamma(img, gamma);
    return img;
}


// ----------- Preview Dialog for Color Levels Adjustment --------- //

enum {
    NO_SLIDER,
    LEFT_SLIDER,
    RIGHT_SLIDER
};

#include <QPainter>
#include <QMouseEvent>

LevelsWidget:: LevelsWidget(QWidget *parent, int l_val, int r_val, QColor clr) : QLabel(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
    drag_slider = NO_SLIDER;
    left_val = l_val;
    right_val = r_val;
    color = clr;
    redraw(left_val, right_val);
}

void
LevelsWidget:: redraw(int l_val, int r_val)
{
    QPixmap pm(256, 32);
    pm.fill(Qt::white);
    QPainter painter(&pm);
    QLinearGradient grad(0,0,255,0);
    grad.setColorAt(0, Qt::black);
    grad.setColorAt(1, color);
    painter.setBrush(QBrush(grad));
    painter.drawRect(0,0, 255, 16);

    // draw left slider
    painter.setBrush(Qt::gray);
    QPoint l_pt(l_val,16);
    QPoint l_pts[] = {l_pt, l_pt+QPoint(-8, 15), l_pt+QPoint(8,15)};
    painter.drawConvexPolygon(l_pts,3);
    // draw right slider
    painter.setBrush(Qt::red);
    QPoint r_pt(r_val,16);
    QPoint r_pts[] = {r_pt, r_pt+QPoint(-8, 15), r_pt+QPoint(8,15)};
    painter.drawConvexPolygon(r_pts,3);
    painter.end();
    setPixmap(pm);
}

void
LevelsWidget:: mousePressEvent(QMouseEvent *ev)
{
    click_pos = ev->pos();
    if (abs(click_pos.x()-right_val)<8)
        drag_slider = RIGHT_SLIDER;
    else if (abs(click_pos.x()-left_val)<8)
        drag_slider = LEFT_SLIDER;
}

void
LevelsWidget:: mouseReleaseEvent(QMouseEvent *ev)
{
    if (drag_slider==LEFT_SLIDER){
        left_val = MAX(left_val + ev->pos().x() - click_pos.x(), 0);
        left_val = MIN(left_val, right_val-8);
    }
    else if (drag_slider==RIGHT_SLIDER){
        right_val = MIN(right_val + ev->pos().x() - click_pos.x(), 255);
        right_val = MAX(left_val+8, right_val);
    }
    drag_slider = NO_SLIDER;
    emit valueChanged();
}

void
LevelsWidget:: mouseMoveEvent(QMouseEvent *ev)
{
    if (drag_slider==NO_SLIDER)
        return;
    if (drag_slider==LEFT_SLIDER){
        int left = MIN(left_val + ev->pos().x() - click_pos.x(), right_val-8);
        redraw(MAX(left, 0), right_val);
    }
    else {
        int right = MAX(right_val + ev->pos().x() - click_pos.x(), left_val+8);
        redraw(left_val, MIN(right, 255));
    }
}


LevelsDialog:: LevelsDialog(QLabel *canvas, QImage img, float scale) : PreviewDialog(canvas,img,scale)
{
    setWindowTitle("Adjust Color Levels");
    QLabel *label0 = new QLabel("Input Levels", this);
    QLabel *label1 = new QLabel("Output Levels", this);
    inputRSlider = new LevelsWidget(this,0,255,Qt::red);
    inputGSlider = new LevelsWidget(this,0,255,Qt::green);
    inputBSlider = new LevelsWidget(this,0,255,Qt::blue);
    outputRSlider = new LevelsWidget(this,0,255,Qt::red);
    outputGSlider = new LevelsWidget(this,0,255,Qt::green);
    outputBSlider = new LevelsWidget(this,0,255,Qt::blue);
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                    QDialogButtonBox::Cancel, Qt::Horizontal, this);
    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(label0, 0,0,1,1);
    layout->addWidget(label1, 0,1,1,1);
    layout->addWidget(inputRSlider, 1,0,1,1);
    layout->addWidget(outputRSlider, 1,1,1,1);
    layout->addWidget(inputGSlider, 2,0,1,1);
    layout->addWidget(outputGSlider, 2,1,1,1);
    layout->addWidget(inputBSlider, 3,0,1,1);
    layout->addWidget(outputBSlider, 3,1,1,1);
    layout->addWidget(btnBox, 4,0,2,2);

    connect(inputRSlider, SIGNAL(valueChanged()), this, SLOT(preview()));
    connect(inputGSlider, SIGNAL(valueChanged()), this, SLOT(preview()));
    connect(inputBSlider, SIGNAL(valueChanged()), this, SLOT(preview()));
    connect(outputRSlider, SIGNAL(valueChanged()), this, SLOT(preview()));
    connect(outputGSlider, SIGNAL(valueChanged()), this, SLOT(preview()));
    connect(outputBSlider, SIGNAL(valueChanged()), this, SLOT(preview()));
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));
}

QImage
LevelsDialog:: getResult(QImage img)
{
    levelImageChannel(img, CHANNEL_R, inputRSlider->left_val, inputRSlider->right_val,
                                    outputRSlider->left_val, outputRSlider->right_val);
    levelImageChannel(img, CHANNEL_G, inputGSlider->left_val, inputGSlider->right_val,
                                    outputGSlider->left_val, outputGSlider->right_val);
    levelImageChannel(img, CHANNEL_B, inputBSlider->left_val, inputBSlider->right_val,
                                    outputBSlider->left_val, outputBSlider->right_val);
    return img;
}
