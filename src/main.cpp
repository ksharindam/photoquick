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
    menu->addAction("GrayScale", this, SLOT(toGrayScale()));
    menu->addAction("Scanned Page", this, SLOT(adaptiveThresh()));
    menu->addAction("Threshold", this, SLOT(toBlacknWhite()));
    menu->addAction("Smooth/Blur...", this, SLOT(blur()));
    menu->addAction("Sharpen", this, SLOT(sharpenImage()));
    menu->addAction("Sigmoidal Contrast", this, SLOT(sigmoidContrast()));
    effectsBtn->setMenu(menu);
    QHBoxLayout *layout = new QHBoxLayout(scrollAreaWidgetContents);
    layout->setContentsMargins(0, 0, 0, 0);
    image = new Image(this, scrollArea);
    layout->addWidget(image);
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
    connect(image, SIGNAL(imageUpdated()), this, SLOT(updateStatus()));
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
          if (image->animation) image->movie()->deleteLater(); // Delete prev animation
          image->setAnimation(anim);
          adjustWindowSize(true);
          statusbar->showMessage(QString("Resolution : %1x%2").arg(image->width()).arg(image->height()));
          disableButtons(true);
        }
    }
    else {                         // For static images
        QPixmap pm = loadImage(filepath);  // Returns an autorotated image
        if (pm.isNull()) return;
        if (image->animation) image->movie()->deleteLater(); // Delete prev animation
        image->scale = getOptimumScale(pm);
        image->setImage(pm);
        adjustWindowSize();
        disableButtons(false);
    }
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
    QPixmap pm;
    if (image->animation)
        pm = image->movie()->currentPixmap();
    else
        pm = image->pic;
    if (pm.isNull()) return;
    int quality = -1;
    if (filepath.endsWith(".jpg", Qt::CaseInsensitive)) {
        QualityDialog *dlg = new QualityDialog(this, pm);
        if (dlg->exec()==QDialog::Accepted){
            quality = dlg->qualitySpin->value();
        }
        else return;
    }
    pm.save(filepath, NULL, quality);
}

void
Window:: resizeImage()
{
    ResizeDialog *dialog = new ResizeDialog(this, image->pic.width(), image->pic.height());
    if (dialog->exec() == 1) {
        QPixmap pm;
        QString img_width = dialog->widthEdit->text();
        QString img_height = dialog->heightEdit->text();
        if ( !img_width.isEmpty() and !img_height.isEmpty() )
            pm = image->pic.scaled(img_width.toInt(), img_height.toInt(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        else if (not img_width.isEmpty())
            pm = image->pic.scaledToWidth(img_width.toInt(), Qt::SmoothTransformation);
        else if (not img_height.isEmpty())
            pm = image->pic.scaledToHeight(img_height.toInt(), Qt::SmoothTransformation);
        else
            return;
        image->setImage(pm);
    }
}

void
Window:: cropImage()
{
    if (not image->crop_mode) {
        image->enableCropMode(true);
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
        connect(lockratio, SIGNAL(toggled(bool)), image, SLOT(lockCropRatio(bool)));
        connect(spinWidth, SIGNAL(valueChanged(double)), image, SLOT(setCropWidth(double)));
        connect(spinHeight, SIGNAL(valueChanged(double)), image, SLOT(setCropHeight(double)));
        connect(cropnowBtn, SIGNAL(clicked()), image, SLOT(cropImage()));
        connect(cropnowBtn, SIGNAL(clicked()), this, SLOT(cancelCropping()));
        connect(cropcancelBtn, SIGNAL(clicked()), this, SLOT(cancelCropping()));
        crop_widgets << lockratio << labelWH << spinWidth << colon << spinHeight << cropnowBtn << cropcancelBtn;
    }
}

void
Window:: cancelCropping()
{
    image->enableCropMode(false);
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
        QPainter painter(&(image->pic));
        QPen pen(Qt::black);
        pen.setWidth(width);
        pen.setJoinStyle(Qt::MiterJoin);
        painter.setPen(pen);
        painter.drawRect(width/2, width/2, image->pic.width()-width, image->pic.height()-width);
        image->showScaled();
    }
}

void
Window:: createPhotoGrid()
{
    GridDialog *dialog = new GridDialog(image->pic, this);
    if (dialog->exec() == 1) {
        image->scale = getOptimumScale(dialog->gridPaper->photo_grid);
        image->setImage(dialog->gridPaper->photo_grid);
        adjustWindowSize();
    }
}

void
Window:: toGrayScale()
{
    QImage img = image->pic.toImage();
    grayScale(img);
    image->setImage(QPixmap::fromImage(img));
}

void
Window:: toBlacknWhite()
{
    QImage img = image->pic.toImage();
    int thresh = calcOtsuThresh(img);
    globalThresh(img, thresh);
    image->setImage(QPixmap::fromImage(img));
}

void
Window:: adaptiveThresh()
{
    QImage img = image->pic.toImage();
    adaptiveIntegralThresh(img);
    image->setImage(QPixmap::fromImage(img));
}

void
Window:: blur()
{
    bool ok;
    int radius = QInputDialog::getInt(this, "Blur Radius", "Enter Blur Radius :",
                                        1/*val*/, 1/*min*/, 30/*max*/, 1/*step*/, &ok);
    if (not ok) return;
    QImage img = image->pic.toImage();
    boxBlur(img, radius);
    image->setImage(QPixmap::fromImage(img));
}

void
Window:: sharpenImage()
{
    QImage img = image->pic.toImage();
    sharpen(img);
    image->setImage(QPixmap::fromImage(img));
}

// Enhance low light images using Sigmoidal Contrast
void
Window:: sigmoidContrast()
{
    QImage img = image->pic.toImage();
    sigmoidalContrast(img, 0.3);
    image->setImage(QPixmap::fromImage(img));
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
    image->zoomBy(6.0/5);
}

void
Window:: zoomOutImage()
{
    image->zoomBy(5.0/6);
}

void
Window:: origSizeImage()
{
    image->scale = 1.0;
    image->showScaled();
}

void
Window:: rotateLeft()
{
    image->rotate(270);
}

void
Window:: rotateRight()
{
    image->rotate(90);
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
Window:: getOptimumScale(QPixmap pixmap)
{
    float scale;
    int img_width = pixmap.width();
    int img_height = pixmap.height();
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
        resize(image->width() + 2*btnboxwidth + 4,
               image->height() + 4+32);
    }
    else {
        resize(image->pixmap()->width() + 2*btnboxwidth + 4,
               image->pixmap()->height() + 4+32);
    }
    move((screen_width - (width() + 2*offset_x) )/2,
              (screen_height - (height() + offset_x + offset_y))/2 );
}

void
Window:: updateStatus()
{
    int width, height;
    if (image->crop_mode) {
        width = round((image->p2.x() - image->p1.x() + 1)/image->scaleW);
        height = round((image->p2.y() - image->p1.y() + 1)/image->scaleH);
    }
    else {
        width = image->pic.width();
        height = image->pic.height();
    }
    QString text = "Resolution : %1x%2 , Scale : %3x";
    statusbar->showMessage(text.arg(width).arg(height).arg(roundOff(image->scale, 2)));
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
        QPixmap pm = QPixmap(":/images/nidhi.jpg");
        win->image->setImage(pm);
        win->adjustWindowSize();
    }
    return app.exec();
}
