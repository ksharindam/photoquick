/*
...........................................................................
|   Copyright (C) 2016-2019 Arindam Chaudhuri <ksharindam@gmail.com>       |
|                                                                          |
|   This program is free software: you can redistribute it and/or modify   |
|   it under the terms of the GNU General Public License as published by   |
|   the Free Software Foundation, either version 3 of the License, or      |
|   (at your option) any later version.                                    |
|                                                                          |
|   This program is distributed in the hope that it will be useful,        |
|   but WITHOUT ANY WARRANTY; without even the implied warranty of         |
|   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          |
|   GNU General Public License for more details.                           |
|                                                                          |
|   You should have received a copy of the GNU General Public License      |
|   along with this program.  If not, see <http://www.gnu.org/licenses/>.  |
...........................................................................
*/
#include "main.h"
#include "dialogs.h"
#include "photogrid.h"
#include "filters.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QFileInfo>
#include <QPainter>
#include <QDesktopWidget>
#include <QMenu>
#include <cmath>

Window:: Window()
{
    setupUi(this);
    QMenu *menu = new QMenu(effectsBtn);
    menu->addAction("Mirror", this, SLOT(mirror()));
    menu->addAction("GrayScale", this, SLOT(toGrayScale()));
    menu->addAction("Scanned Page", this, SLOT(adaptiveThresh()));
    menu->addAction("Threshold", this, SLOT(toBlacknWhite()));
    menu->addAction("Sharpen", this, SLOT(sharpenImage()));
    menu->addAction("Smooth/Blur...", this, SLOT(blur()));
    menu->addAction("Despeckle", this, SLOT(reduceSpeckleNoise()));
    menu->addAction("Reduce Noise", this, SLOT(reduceImageNoise()));
    menu->addAction("Enhance Contrast", this, SLOT(sigmoidContrast()));
    menu->addAction("White Balance", this, SLOT(whiteBalance()));
    effectsBtn->setMenu(menu);
    QHBoxLayout *layout = new QHBoxLayout(scrollAreaWidgetContents);
    layout->setContentsMargins(0, 0, 0, 0);
    canvas = new Canvas(this, scrollArea);
    layout->addWidget(canvas);
    slideShowBtn->setCheckable(true);
    timer = new QTimer(this);
    connectSignals();
    // Initialize Variables
    QDesktopWidget *desktop = QApplication::desktop();
    screen_width = desktop->availableGeometry().width();
    screen_height = desktop->availableGeometry().height();
    filepath = QString("nidhi.jpg");
    offset_x = settings.value("OffsetX", 4).toInt();
    offset_y = settings.value("OffsetY", 26).toInt();
    btnboxwidth = settings.value("BtnBoxWidth", 60).toInt();
}

void
Window:: connectSignals()
{
    // For the buttons of the left side
    connect(openBtn, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(saveBtn, SIGNAL(clicked()), this, SLOT(saveFile()));
    connect(resizeBtn, SIGNAL(clicked()), this, SLOT(resizeImage()));
    connect(cropBtn, SIGNAL(clicked()), this, SLOT(cropImage()));
    connect(addBorderBtn, SIGNAL(clicked()), this, SLOT(addBorder()));
    connect(photoGridBtn, SIGNAL(clicked()), this, SLOT(createPhotoGrid()));
    connect(quitBtn, SIGNAL(clicked()), this, SLOT(close()));
    // For the buttons of the right side
    connect(prevBtn, SIGNAL(clicked()), this, SLOT(openPrevImage()));
    connect(nextBtn, SIGNAL(clicked()), this, SLOT(openNextImage()));
    connect(zoomInBtn, SIGNAL(clicked()), this, SLOT(zoomInImage()));
    connect(zoomOutBtn, SIGNAL(clicked()), this, SLOT(zoomOutImage()));
    connect(origSizeBtn, SIGNAL(clicked()), this, SLOT(origSizeImage()));
    connect(rotateLeftBtn, SIGNAL(clicked()), this, SLOT(rotateLeft()));
    connect(rotateRightBtn, SIGNAL(clicked()), this, SLOT(rotateRight()));
    connect(slideShowBtn, SIGNAL(clicked(bool)), this, SLOT(playSlideShow(bool)));
    connect(timer, SIGNAL(timeout()), this, SLOT(openNextImage()));
    // Connect other signals
    connect(canvas, SIGNAL(imageUpdated()), this, SLOT(updateStatus()));
}

void
Window:: openFile()
{
    QString filefilter = "Image files (*.jpg *.png *.jpeg *.svg *.gif *.tiff *.ppm *.bmp);;JPEG Images (*.jpg *.jpeg);;"
                         "PNG Images (*.png);;SVG Images (*.svg);;All Files (*)";
    QString filepath = QFileDialog::getOpenFileName(this, "Open Image", this->filepath, filefilter);
    if (filepath.isEmpty()) return;
    openImage(filepath);
}

void
Window:: openImage(QString filepath)
{
    QImageReader img_reader(filepath);
    if (QString::fromUtf8(img_reader.format()).compare("gif")==0) { // For gif animations
        QMovie *anim = new QMovie(filepath, QByteArray(), this);
        if (anim->isValid()) {
          canvas->setAnimation(anim);
          adjustWindowSize(true);
          statusbar->showMessage(QString("Resolution : %1x%2").arg(canvas->width()).arg(canvas->height()));
          disableButtons(true);
        }
    }
    else {                         // For static images
        QImage img = loadImage(filepath);  // Returns an autorotated image
        if (img.isNull()) return;
        canvas->scale = getOptimumScale(img);
        canvas->setImage(img);
        adjustWindowSize();
        disableButtons(false);
    }
    if (QString::fromUtf8(img_reader.format()).compare("png")==0)
        canvas->has_alpha = true;
    else
        canvas->has_alpha = false;
    this->filepath = filepath;
    setWindowTitle(QFileInfo(filepath).fileName());
}

void
Window:: saveFile()
{
    QString filefilter = QString("Image files (*.jpg *.png *.jpeg *.ppm *.bmp *.tiff);;"
                                 "JPEG Image (*.jpg);;PNG Image (*.png);;Tagged Image (*.tiff);;"
                                 "Portable Pixmap (*.ppm);;X11 Pixmap (*.xpm);;Windows Bitmap (*.bmp)");
    QString filepath = QFileDialog::getSaveFileName(this, "Save Image", this->filepath, filefilter);
    if (filepath.isEmpty()) return;
    QImage img = canvas->image;
    if (canvas->animation)
        img = canvas->movie()->currentImage();
    if (img.isNull()) return;
    int quality = -1;
    if (filepath.endsWith(".jpg", Qt::CaseInsensitive)) {
        if (canvas->has_alpha) { // converts background to white
            img = QImage(img.width(), img.height(), QImage::Format_ARGB32);
            img.fill(Qt::white);
            QPainter painter(&img);
            painter.drawImage(0,0, canvas->image);
            painter.end();
        }
        QualityDialog *dlg = new QualityDialog(this, img);
        if (dlg->exec()==QDialog::Accepted){
            quality = dlg->qualitySpin->value();
        }
        else return;
    }
    img.save(filepath, NULL, quality);
}

void
Window:: resizeImage()
{
    ResizeDialog *dialog = new ResizeDialog(this, canvas->image.width(), canvas->image.height());
    if (dialog->exec() == 1) {
        QImage img;
        QString img_width = dialog->widthEdit->text();
        QString img_height = dialog->heightEdit->text();
        if ( !img_width.isEmpty() and !img_height.isEmpty() )
            img = canvas->image.scaled(img_width.toInt(), img_height.toInt(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        else if (not img_width.isEmpty())
            img = canvas->image.scaledToWidth(img_width.toInt(), Qt::SmoothTransformation);
        else if (not img_height.isEmpty())
            img = canvas->image.scaledToHeight(img_height.toInt(), Qt::SmoothTransformation);
        else
            return;
        canvas->setImage(img);
    }
}

void
Window:: cropImage()
{
    if (not canvas->crop_mode) {
        canvas->enableCropMode(true);
        QCheckBox *lockratio = new QCheckBox("Lock Ratio  ", statusbar);
        statusbar->addPermanentWidget(lockratio);
        QLabel *labelWH = new QLabel("<b>W:H =</b>", statusbar);
        statusbar->addPermanentWidget(labelWH);
        labelWH->setEnabled(false);
        QDoubleSpinBox *spinWidth = new QDoubleSpinBox(statusbar);
        spinWidth->setRange(0.1, 9.9);
        spinWidth->setSingleStep(0.1);
        spinWidth->setDecimals(1);
        spinWidth->setMaximumWidth(44);
        spinWidth->setValue(3.5);
        spinWidth->setEnabled(false);
        statusbar->addPermanentWidget(spinWidth);
        QLabel *colon = new QLabel(":", statusbar);
        statusbar->addPermanentWidget(colon);
        QDoubleSpinBox *spinHeight = new QDoubleSpinBox(statusbar);
        spinHeight->setRange(0.1, 9.9);
        spinHeight->setSingleStep(0.1);
        spinHeight->setDecimals(1);
        spinHeight->setMaximumWidth(44);
        spinHeight->setValue(4.5);
        spinHeight->setEnabled(false);
        statusbar->addPermanentWidget(spinHeight);
        QPushButton *cropnowBtn = new QPushButton("Crop Now", statusbar);
        statusbar->addPermanentWidget(cropnowBtn);
        QPushButton *cropcancelBtn = new QPushButton("Cancel", statusbar);
        statusbar->addPermanentWidget(cropcancelBtn);
        connect(lockratio, SIGNAL(toggled(bool)), labelWH, SLOT(setEnabled(bool)));
        connect(lockratio, SIGNAL(toggled(bool)), spinWidth, SLOT(setEnabled(bool)));
        connect(lockratio, SIGNAL(toggled(bool)), spinHeight, SLOT(setEnabled(bool)));
        connect(lockratio, SIGNAL(toggled(bool)), canvas, SLOT(lockCropRatio(bool)));
        connect(spinWidth, SIGNAL(valueChanged(double)), canvas, SLOT(setCropWidth(double)));
        connect(spinHeight, SIGNAL(valueChanged(double)), canvas, SLOT(setCropHeight(double)));
        connect(cropnowBtn, SIGNAL(clicked()), canvas, SLOT(cropImage()));
        connect(cropnowBtn, SIGNAL(clicked()), this, SLOT(cancelCropping()));
        connect(cropcancelBtn, SIGNAL(clicked()), this, SLOT(cancelCropping()));
        crop_widgets << lockratio << labelWH << spinWidth << colon << spinHeight << cropnowBtn << cropcancelBtn;
    }
}

void
Window:: cancelCropping()
{
    canvas->enableCropMode(false);
    while (not crop_widgets.isEmpty()) {
        QWidget *widget = crop_widgets.takeLast();
        statusbar->removeWidget(widget);
        widget->deleteLater();
    }
}

void
Window:: addBorder()
{
    bool ok;
    int width = QInputDialog::getInt(this, "Add Border", "Enter Border Width :", 2, 1, 100, 1, &ok);
    if (ok) {
        QPainter painter(&(canvas->image));
        QPen pen(Qt::black);
        pen.setWidth(width);
        pen.setJoinStyle(Qt::MiterJoin);
        painter.setPen(pen);
        painter.drawRect(width/2, width/2, canvas->image.width()-width, canvas->image.height()-width);
        canvas->showScaled();
    }
}

void
Window:: createPhotoGrid()
{
    GridDialog *dialog = new GridDialog(canvas->image, this);
    if (dialog->exec() == 1) {
        canvas->scale = getOptimumScale(dialog->gridPaper->photo_grid);
        canvas->setImage(dialog->gridPaper->photo_grid);
        adjustWindowSize();
    }
}

void
Window:: toGrayScale()
{
    grayScale(canvas->image);
    canvas->showScaled();
}

void
Window:: toBlacknWhite()
{
    int thresh = calcOtsuThresh(canvas->image);
    threshold(canvas->image, thresh);
    canvas->showScaled();
}

void
Window:: adaptiveThresh()
{
    adaptiveThreshold(canvas->image);
    canvas->showScaled();
}

void
Window:: blur()
{
    bool ok;
    int radius = QInputDialog::getInt(this, "Blur Radius", "Enter Blur Radius :",
                                        1/*val*/, 1/*min*/, 30/*max*/, 1/*step*/, &ok);
    if (not ok) return;
    //gaussianBlur(canvas->image, radius);
    boxFilter(canvas->image, radius);
    canvas->showScaled();
}

void
Window:: sharpenImage()
{
    sharpen(canvas->image);
    canvas->showScaled();
}

void
Window:: reduceSpeckleNoise()
{
    despeckle(canvas->image);
    canvas->showScaled();
}

void
Window:: reduceImageNoise()
{
    medianFilter(canvas->image, 1);
    //reduceNoise(canvas->image, 1);
    canvas->showScaled();
}

// Enhance low light images using Sigmoidal Contrast
void
Window:: sigmoidContrast()
{
    sigmoidalContrast(canvas->image, 0.3);
    canvas->showScaled();
}

void
Window:: whiteBalance()
{
    autoWhiteBalance(canvas->image);
    canvas->showScaled();
}

void
Window:: openPrevImage()
{
    QFileInfo fi(filepath);
    if (not fi.exists()) return;
    QString filename = fi.fileName();
    QString basedir = fi.absolutePath();         // This does not include filename
    QString file_filter("*.jpg *.jpeg *.png *.gif *.svg *.bmp *.tiff");
    QStringList image_list = fi.dir().entryList(file_filter.split(" "));

    int index = image_list.indexOf(filename);
    if (index==0) index = image_list.count();
    QString prevfile = image_list[index-1];
    openImage(basedir + "/" + prevfile);
}

void
Window:: openNextImage()
{
    QFileInfo fi(filepath);
    if (not fi.exists()) return;
    QString filename = fi.fileName();
    QString basedir = fi.absolutePath();         // This does not include filename
    QString file_filter("*.jpg *.jpeg *.png *.gif *.svg *.bmp *.tiff");
    QStringList image_list = fi.dir().entryList(file_filter.split(" "));

    int index = image_list.indexOf(filename);
    if (index == image_list.count()-1) index = -1;
    QString nextfile = image_list[index+1];
    openImage(basedir + '/' + nextfile);
}

void
Window:: zoomInImage()
{
    canvas->zoomBy(6.0/5);
}

void
Window:: zoomOutImage()
{
    canvas->zoomBy(5.0/6);
}

void
Window:: origSizeImage()
{
    canvas->scale = 1.0;
    canvas->showScaled();
}

void
Window:: rotateLeft()
{
    canvas->rotate(270);
}

void
Window:: rotateRight()
{
    canvas->rotate(90);
}

void
Window:: mirror()
{
    canvas->rotate(180, Qt::YAxis);
}

void
Window:: playSlideShow(bool checked)
{
    if (checked) { // Start slideshow
        timer->start(3000);
        slideShowBtn->setIcon(QIcon(":/images/pause.png"));
    }
    else {       // Stop slideshow
        timer->stop();
        slideShowBtn->setIcon(QIcon(":/images/play.png"));
    }
}

float
Window:: getOptimumScale(QImage img)
{
    float scale;
    int img_width = img.width();
    int img_height = img.height();
    int max_width = screen_width - (2*btnboxwidth + 2*offset_x);
    int max_height = screen_height - (offset_y + offset_x + 4+32); // 32 for statusbar with buttons
    if ((img_width > max_width) || (img_height > max_height)) {
        if (float(max_width)/max_height > float(img_width)/img_height) {
            scale = float(max_height)/img_height;
        }
        else
            scale = float(max_width)/img_width;
    }
    else
        scale = 1.0;
    return scale;
}

void
Window:: adjustWindowSize(bool animation)
{
    if (isMaximized()) return;
    if (animation) {
        waitFor(30);        // Wait little to let Label resize and get correct width height
        resize(canvas->width() + 2*btnboxwidth + 4,
               canvas->height() + 4+32);
    }
    else {
        resize(canvas->pixmap()->width() + 2*btnboxwidth + 4,
               canvas->pixmap()->height() + 4+32);
    }
    move((screen_width - (width() + 2*offset_x) )/2,
              (screen_height - (height() + offset_x + offset_y))/2 );
}

void
Window:: updateStatus()
{
    int width, height;
    if (canvas->crop_mode) {
        width = round((canvas->p2.x() - canvas->p1.x() + 1)/canvas->scaleW);
        height = round((canvas->p2.y() - canvas->p1.y() + 1)/canvas->scaleH);
    }
    else {
        width = canvas->image.width();
        height = canvas->image.height();
    }
    QString text = "Resolution : %1x%2 , Scale : %3x";
    statusbar->showMessage(text.arg(width).arg(height).arg(roundOff(canvas->scale, 2)));
}

void
Window:: disableButtons(bool disable)
{
    resizeBtn->setDisabled(disable);
    cropBtn->setDisabled(disable);
    addBorderBtn->setDisabled(disable);
    photoGridBtn->setDisabled(disable);
    zoomInBtn->setDisabled(disable);
    zoomOutBtn->setDisabled(disable);
    origSizeBtn->setDisabled(disable);
    rotateLeftBtn->setDisabled(disable);
    rotateRightBtn->setDisabled(disable);
}

void
Window:: closeEvent(QCloseEvent *ev)
{
    settings.setValue("OffsetX", geometry().x()-x());
    settings.setValue("OffsetY", geometry().y()-y());
    settings.setValue("BtnBoxWidth", frame->width());
    QMainWindow::closeEvent(ev);
}


// Static functions
void waitFor(int millisec)
{
    // Creates an eventloop to wait for a time
    QEventLoop *loop = new QEventLoop();
    QTimer::singleShot(millisec, loop, SLOT(quit()));
    loop->exec();
    loop->deleteLater();
}

float roundOff(float num, int dec)
{
    double m = (num < 0.0) ? -1.0 : 1.0;   // check if input is negative
    double pwr = pow(10, dec);
    return float(floor((double)num * m * pwr + 0.5) / pwr) * m;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("qmageview");
    app.setApplicationName("qmageview");
    Window *win = new Window();
    win->show();
    if (argc > 1) {
        QString path = QString::fromUtf8(argv[1]);
        QFileInfo fileinfo(path);
        if (fileinfo.exists())
            win->openImage(path);
    }
    else {
        QImage img = QImage(":/images/nidhi.jpg");
        win->canvas->setImage(img);
        win->adjustWindowSize();
    }
    return app.exec();
}
