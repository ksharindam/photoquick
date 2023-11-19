// this file is part of photoquick program which is GPLv3 licensed
#include "dialogs.h"
#include "common.h"
#include "filters.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QGridLayout>
#include <QColorDialog>// for BgColorDialog
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QPainter>// for LevelsDialog
#include <QMouseEvent>
#include <cmath>

// ------------ Dialog to set JPG Options for saving ------------

JpegDialog:: JpegDialog(QWidget *parent, QImage &img) : QDialog(parent), image(img)
{
    setWindowTitle("JPEG Options");
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
    sizeLabel = new QLabel("Wait...", this);
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,
                                                    Qt::Horizontal, this);
    QCheckBox *saveDpiCheck = new QCheckBox("Save DPI :", this);
    dpiSpin = new QSpinBox(this);
    dpiSpin->setAlignment(Qt::AlignHCenter);
    dpiSpin->setSingleStep(50);
    dpiSpin->setRange(50, 1200);
    dpiSpin->setValue(300);

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(qualityLabel, 0,0, 1,1);
    layout->addWidget(qualitySpin, 0,1, 1,1);
    layout->addWidget(showSizeCheck, 1,0, 1,1);
    layout->addWidget(sizeLabel, 1,1, 1,1);
    layout->addWidget(saveDpiCheck, 2,0, 1,1);
    layout->addWidget(dpiSpin, 2,1, 1,1);
    layout->addWidget(btnBox, 3,0, 2,2);
    sizeLabel->hide();
    dpiSpin->setEnabled(false);

    connect(showSizeCheck, SIGNAL(clicked(bool)), this, SLOT(toggleCheckSize(bool)));
    connect(saveDpiCheck, SIGNAL(clicked(bool)), dpiSpin, SLOT(setEnabled(bool)));
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void
JpegDialog:: toggleCheckSize(bool checked)
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
JpegDialog:: checkFileSize()
{
    int filesize = getJpgFileSize(image, qualitySpin->value());
    QString text = "%1 KB";
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
    QLabel *label = new QLabel("Expand sides by :", this);
    widthSpin = new QSpinBox(this);
    widthSpin->setAlignment(Qt::AlignHCenter);
    widthSpin->setSuffix(" px");
    widthSpin->setRange(1, border_w*5);
    widthSpin->setValue(border_w);
    QLabel *label2 = new QLabel("Set Border Type :", this);
    combo = new QComboBox(this);
    QStringList items = {"Clone Edges", "White Color", "Black Color", "Other Color"};
    combo->addItems(items);
    QCheckBox *allSidesCheck = new QCheckBox("All Sides", this);
    allSidesCheck->setChecked(true);
    sidesFrame = new QWidget(this);
    QGridLayout *sidesLayout = new QGridLayout(sidesFrame);
    leftCheckBox = new QCheckBox("Left", this);
    rightCheckBox = new QCheckBox("Right", this);
    topCheckBox = new QCheckBox("Top", this);
    bottomCheckBox = new QCheckBox("Bottom", this);
    sidesLayout->addWidget(topCheckBox, 0,1, 1,1);
    sidesLayout->addWidget(leftCheckBox, 1,0, 1,1);
    sidesLayout->addWidget(rightCheckBox, 1,2, 1,1);
    sidesLayout->addWidget(bottomCheckBox, 2,1, 1,1);
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal, this);
    vLayout->addWidget(label);
    vLayout->addWidget(widthSpin);
    vLayout->addWidget(label2);
    vLayout->addWidget(combo);
    vLayout->addWidget(allSidesCheck);
    vLayout->addWidget(sidesFrame);
    vLayout->addWidget(btnBox);
    connect(allSidesCheck, SIGNAL(clicked(bool)), this, SLOT(toggleAllSides(bool)));
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));
    leftCheckBox->setChecked(true);
    rightCheckBox->setChecked(true);
    topCheckBox->setChecked(true);
    bottomCheckBox->setChecked(true);
    sidesFrame->setHidden(true);
}

void
ExpandBorderDialog:: toggleAllSides(bool checked)
{
    sidesFrame->setHidden(checked);
    this->adjustSize();
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
    QTimer::singleShot(30, this, SLOT(repositionWindow()));
}

void
PreviewDialog:: repositionWindow()
{
    // move to bottom edge of Canvas, so that preview dialog does not guard the canvas
    QPoint canvas_pos = canvas->mapToGlobal(QPoint(0,0));
    int canvas_bottom = canvas_pos.y() + canvas->height();
    QPoint pos = this->mapToGlobal(QPoint(0,0));
    int bottom = pos.y() + height();
    this->move(pos.x(), this->y()+ canvas_bottom - bottom);
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
    angleSpin = new QDoubleSpinBox(this);
    angleSpin->setDecimals(1);
    angleSpin->setSingleStep(0.5);
    angleSpin->setRange(-90.0, 90.0);
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                    QDialogButtonBox::Cancel, Qt::Horizontal, this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label0);
    layout->addWidget(angleSpin);
    layout->addWidget(btnBox);

    connect(angleSpin, SIGNAL(valueChanged(double)), this, SLOT(triggerPreview()));
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


LevelsWidget:: LevelsWidget(QWidget *parent, int l_val, int r_val, QColor clr) : QLabel(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
    drag_slider = NO_SLIDER;
    left_val = l_val;
    right_val = r_val;
    color = clr;
    redraw();
}

void
LevelsWidget:: redraw()
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
    QPoint l_pt(left_val,16);
    QPoint l_pts[] = {l_pt, l_pt+QPoint(-8, 15), l_pt+QPoint(8,15)};
    painter.drawConvexPolygon(l_pts,3);
    // draw right slider
    painter.setBrush(Qt::red);
    QPoint r_pt(right_val,16);
    QPoint r_pts[] = {r_pt, r_pt+QPoint(-8, 15), r_pt+QPoint(8,15)};
    painter.drawConvexPolygon(r_pts,3);
    painter.end();
    setPixmap(pm);
}

void
LevelsWidget:: setLeftValue(int val)
{
    left_val = MAX(0, val);
    left_val = MIN(left_val, right_val-8);
}

void
LevelsWidget:: setRightValue(int val)
{
    right_val = MIN(val, 255);
    right_val = MAX(left_val+8, right_val);
}

void
LevelsWidget:: mousePressEvent(QMouseEvent *ev)
{
    int pos = ev->pos().x();
    if (abs(pos-right_val)<8){
        drag_slider = RIGHT_SLIDER;
        offset = right_val-pos;// offset of val from mouse pointer
    }
    else if (abs(pos-left_val)<8){
        drag_slider = LEFT_SLIDER;
        offset = left_val-pos;
    }
}

void
LevelsWidget:: mouseReleaseEvent(QMouseEvent *)
{
    drag_slider = NO_SLIDER;
    emit valueChanged();
}

void
LevelsWidget:: mouseMoveEvent(QMouseEvent *ev)
{
    if (drag_slider==NO_SLIDER)
        return;
    if (drag_slider==LEFT_SLIDER){
        setLeftValue(ev->pos().x() + offset);
    }
    else {
        setRightValue(ev->pos().x() + offset);
    }
    redraw();
}

void
LevelsWidget:: wheelEvent(QWheelEvent *ev)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    int inc = ev->delta() > 0 ? 2 : -2;
#else
    int inc = ev->angleDelta().y() > 0 ? 2 : -2;
#endif
    if (ev->pos().x()>127)
        setRightValue(right_val+inc);
    else
        setLeftValue(left_val+inc);

    redraw();
    emit valueChanged();
    ev->accept();
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



// ----------- Preview Dialog for Adding background color to transparent images --------- //

BgColorDialog:: BgColorDialog(QLabel *canvas, QImage img, float scale) : PreviewDialog(canvas,img,scale)
{
    this->resize(250, 100);
    this->setWindowTitle("Add Background Color");
    color_map = {
        {"White", 0xffffff},
        {"Black", 0xff000000},
        {"Coral Blue", 0xafdcec},
        {"Day Sky Blue", 0x82caff},
        {"Apache", 0xdfbe6f},
        {"Pink Flare", 0xe1c0c8},
    };
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    QLabel *label = new QLabel("Select Background Color :", this);
    combo = new QComboBox(this);
    combo->addItems({"White", "Black", "Coral Blue", "Day Sky Blue", "Apache",
            "Pink Flare", "Transparent", "Other"});
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal, this);
    vLayout->addWidget(label);
    vLayout->addWidget(combo);
    vLayout->addWidget(btnBox);
    connect(combo, SIGNAL(activated(const QString&)), this, SLOT(onColorChange(const QString&)));
    connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));
    preview();
}

void
BgColorDialog:: onColorChange(const QString& color_name)
{
    if (color_map.count(color_name)>0){
        bg_color = color_map[color_name];
    }
    else if (color_name=="Other") {
        QColor clr = QColorDialog::getColor(QColor(bg_color), this);
        if (clr.isValid())
            bg_color = clr.rgb();
    }
    else {// keep transparency
        bg_color = 0x00000000;
    }
    preview();
}

QImage
BgColorDialog:: getResult(QImage img)
{
    return setImageBackgroundColor(img, bg_color);
}

void
BgColorDialog:: selectColorName(QString color_name)
{
    int index = combo->findText(color_name);
    if (index==-1)
        return;
    combo->setCurrentIndex(index);
    onColorChange(color_name);
}


// -----------------   Update Manager Dialog ------------------- //

// check if versionA is later than versionB (versions must be in x.x.x format)
static bool isLaterThan(QString versionA, QString versionB)
{
    QStringList listA = versionA.split(".");
    QStringList listB = versionB.split(".");
    for (int i=0; i<3; i++) {
        if (listA[i].toInt() == listB[i].toInt())
            continue;
        return listA[i].toInt() > listB[i].toInt();
    }
    return false;
}

UpdateDialog:: UpdateDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Update PhotoQuick");
    QGridLayout *layout = new QGridLayout(this);
    currentVersionLabel = new QLabel(QString("Current Version : %1").arg(PROG_VERSION), this);
    latestVersionLabel = new QLabel("Latest Release : x.x.x", this);
    textView = new QTextEdit(this);
    textView->setReadOnly(true);
    updateBtn = new QPushButton("Check for Update", this);
    closeBtn = new QPushButton("Cancel", this);
    buttonBox = new QWidget(this);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonBox);
    buttonLayout->addStretch();
    buttonLayout->addWidget(updateBtn);
    buttonLayout->addWidget(closeBtn);

    layout->addWidget(currentVersionLabel, 0,0,1,1);
    layout->addWidget(latestVersionLabel, 1,0,1,1);
    layout->addWidget(textView, 2,0,1,1);
    layout->addWidget(buttonBox, 3,0,1,1);

    connect(closeBtn, SIGNAL(clicked()), this, SLOT(reject()));
    connect(updateBtn, SIGNAL(clicked()), this, SLOT(checkForUpdate()));

    textView->hide();
}


void
UpdateDialog:: checkForUpdate()
{
    if (not latest_version.isEmpty()) {
        return download();
    }
    updateBtn->setEnabled(false);
    // show textView and enlarge window and place to center
    int win_w = this->width();
    int win_h = this->height();
    textView->show();
    move(this->pos() - QPoint((500-win_w)/2, (300-win_h)/2));// place center
    resize(500,300);
    textView->setPlainText("Checking for Update...");
    waitFor(100);

    QString url("https://api.github.com/repos/ksharindam/photoquick/releases/latest");
    QString info_file =  desktopPath() + "/photoquick.json";

#ifdef _WIN32
    if (QProcess::execute("certutil", {"-urlcache", "-split", "-f", url, info_file})!=0){
#else
    if (QProcess::execute("wget", {"--inet4-only", "-O", info_file, url})!=0){// ipv4 connects faster
#endif
        QFile::remove(info_file);// 0 byte photoquick.json file remains if wget fails
        textView->setPlainText("Failed to connect !\nCheck your internet connection.");
        updateBtn->setEnabled(true);
        return;
    }

    QFile file(info_file);
    if (!file.open(QFile::ReadOnly)){
        textView->setPlainText("Error ! Failed to open latest release info file.");
        return;
    }
    QTextStream stream(&file);
    QString text(stream.readAll());
    file.close();
    file.remove();

    int pos = text.indexOf("\"tag_name\"");// parse "tag_name": "v4.4.2"
    if (pos >= 0) {
        int begin = text.indexOf("\"", pos+10) + 2;
        int end = text.indexOf("\"", begin);
        latest_version = text.mid(begin, end-begin);
        latestVersionLabel->setText(QString("Latest Release : %1").arg(latest_version));
    }
    if (not latest_version.isEmpty()) {
        if (isLaterThan(latest_version, PROG_VERSION)) {// latest version is available
            // body contains changelog and release info
            pos = text.indexOf("\"body\"");// parse "body": "### Changelog\r\n4.4.1 : fixed bug \r\n"
            if (pos >= 0) {
                int begin = text.indexOf("\"", pos+6) + 1;
                int end = text.indexOf("\"", begin);
                QString body = text.mid(begin, end-begin);
                textView->setPlainText(body.split("\\r\\n").join("\n"));
            }
            updateBtn->setText("Download");
        }
        else {
            latest_version = "";
            textView->setPlainText("You are already using the latest version");
        }
    }
    else {
        textView->setPlainText("Error ! Unable to parse release version.");
    }
    updateBtn->setEnabled(true);
}

void
UpdateDialog:: download()
{
#ifdef _WIN32
    QString filename = QString("PhotoQuick-%1.exe").arg(latest_version);// eg. PhotoQuick-4.4.2.exe
#else
    // currently we provide PhotoQuick-x86_64.AppImage and PhotoQuick-armhf.AppImage
    QString filename = QString("PhotoQuick-%1.AppImage").arg(ARCH);
#endif
    QString addr("https://github.com/ksharindam/photoquick/releases/latest/download/%1");
    QDesktopServices::openUrl(QUrl(addr.arg(filename)));
}
