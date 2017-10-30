#include "main.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QFileInfo>
#include <QPainter>
#include <QDesktopWidget>
#include <cmath>

Window:: Window()
{
    setupUi(this);
    QHBoxLayout *layout = new QHBoxLayout(scrollAreaWidgetContents);
    layout->setContentsMargins(0, 0, 0, 0);
    image = new Image(this);
    layout->addWidget(image);
    slideShowBtn->setCheckable(true);
    timer = new QTimer(this);
    connectSignals();
    // Initialize Variables
    QDesktopWidget *desktop = QApplication::desktop();
    screen_width = desktop->availableGeometry().width();
    screen_height = desktop->availableGeometry().height();
    filepath = QString("");
    offset_x = settings.value("OffsetX", 4).toInt();
    offset_y = settings.value("OffsetY", 26).toInt();
    btnboxwidth = settings.value("BtnBoxWidth", 60).toInt();
}

void
Window:: connectSignals()
{
    // For the buttons of the left side
    QObject::connect(openBtn, SIGNAL(clicked()), this, SLOT(openFile()));
    QObject::connect(saveBtn, SIGNAL(clicked()), this, SLOT(saveFile()));
    QObject::connect(resizeBtn, SIGNAL(clicked()), this, SLOT(resizeImage()));
    QObject::connect(cropBtn, SIGNAL(clicked()), this, SLOT(cropImage()));
    QObject::connect(addBorderBtn, SIGNAL(clicked()), this, SLOT(addBorder()));
    QObject::connect(photoGridBtn, SIGNAL(clicked()), this, SLOT(createPhotoGrid()));
    QObject::connect(quitBtn, SIGNAL(clicked()), this, SLOT(close()));
    // For the buttons of the right side
    QObject::connect(prevBtn, SIGNAL(clicked()), this, SLOT(openPrevImage()));
    QObject::connect(nextBtn, SIGNAL(clicked()), this, SLOT(openNextImage()));
    QObject::connect(zoomInBtn, SIGNAL(clicked()), this, SLOT(zoomInImage()));
    QObject::connect(zoomOutBtn, SIGNAL(clicked()), this, SLOT(zoomOutImage()));
    QObject::connect(origSizeBtn, SIGNAL(clicked()), this, SLOT(origSizeImage()));
    QObject::connect(rotateLeftBtn, SIGNAL(clicked()), this, SLOT(rotateLeft()));
    QObject::connect(rotateRightBtn, SIGNAL(clicked()), this, SLOT(rotateRight()));
    QObject::connect(slideShowBtn, SIGNAL(clicked(bool)), this, SLOT(playSlideShow(bool)));
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(openNextImage()));
    // Connect other signals
    QObject::connect(image, SIGNAL(imageUpdated()), this, SLOT(updateStatus()));
    // Connect Shortcuts
    openBtn->setShortcut(QString("Ctrl+O"));
    saveBtn->setShortcut(QString("Ctrl+S"));
    resizeBtn->setShortcut(QString("Ctrl+R"));
    cropBtn->setShortcut(QString("Ctrl+X"));
    addBorderBtn->setShortcut(QString("Ctrl+B"));
    photoGridBtn->setShortcut(QString("Ctrl+G"));
    quitBtn->setShortcut(QString("Esc"));
    prevBtn->setShortcut(QString("Left"));
    nextBtn->setShortcut(QString("Right"));
    zoomInBtn->setShortcut(QString("+"));
    zoomOutBtn->setShortcut(QString("-"));
    origSizeBtn->setShortcut(QString("1"));
    rotateLeftBtn->setShortcut(QString("Ctrl+Left"));
    rotateRightBtn->setShortcut(QString("Ctrl+Right"));
    slideShowBtn->setShortcut(QString("Space"));
}

void
Window:: openFile()
{
    QString filefilter = "Image files (*.jpg *.png *.jpeg *.svg *.gif);;JPEG Images (*.jpg *.jpeg);;All Files (*)";
    QString filepath = QFileDialog::getOpenFileName(this, "Open Image", this->filepath, filefilter); 
    if (filepath.isEmpty()) return;
    openImage(filepath);
}

void
Window:: openImage(QString filepath)
{
    if (filepath.endsWith(".gif")) { // For gif animations
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
    QPixmap pm;
    QString filefilter = QString("Image files (*.jpg *.png *.jpeg);;JPEG Images (*.jpg *.jpeg)");
    QString filepath = QFileDialog::getSaveFileName(this, "Save Image", this->filepath, filefilter);
    if (not filepath.isEmpty()) {
        int quality = -1;
        /*if sel_filter=='JPEG Images (*.jpg *.jpeg)':
            val, ok = QInputDialog.getInt(self, "Set Quality", "Set Image Quality (%) :", 75, 10, 100)
            if ok : quality = val*/
        if (image->animation)
            pm = image->movie()->currentPixmap();
        else
            pm = image->pic;
        if (not pm.isNull())
            pm.save(filepath, NULL, quality);
    }
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
        spinWidth->setMaximumWidth(40);
        spinWidth->setValue(3.5);
        spinWidth->setEnabled(false);
        statusbar->addPermanentWidget(spinWidth);
        QLabel *colon = new QLabel(":", statusbar);
        statusbar->addPermanentWidget(colon);
        QDoubleSpinBox *spinHeight = new QDoubleSpinBox(statusbar);
        spinHeight->setRange(0.1, 9.9);
        spinHeight->setSingleStep(0.1);
        spinHeight->setDecimals(1);
        spinHeight->setMaximumWidth(40);
        spinHeight->setValue(4.5);
        spinHeight->setEnabled(false);
        statusbar->addPermanentWidget(spinHeight);
        QPushButton *cropnowBtn = new QPushButton("Crop Now", statusbar);
        statusbar->addPermanentWidget(cropnowBtn);
        QPushButton *cropcancelBtn = new QPushButton("Cancel", statusbar);
        statusbar->addPermanentWidget(cropcancelBtn);
        QObject::connect(lockratio, SIGNAL(toggled(bool)), labelWH, SLOT(setEnabled(bool)));
        QObject::connect(lockratio, SIGNAL(toggled(bool)), spinWidth, SLOT(setEnabled(bool)));
        QObject::connect(lockratio, SIGNAL(toggled(bool)), spinHeight, SLOT(setEnabled(bool)));
        QObject::connect(lockratio, SIGNAL(toggled(bool)), image, SLOT(lockCropRatio(bool)));
        QObject::connect(spinWidth, SIGNAL(valueChanged(double)), image, SLOT(setCropWidth(double)));
        QObject::connect(spinHeight, SIGNAL(valueChanged(double)), image, SLOT(setCropHeight(double)));
        QObject::connect(cropnowBtn, SIGNAL(clicked()), image, SLOT(cropImage()));
        QObject::connect(cropnowBtn, SIGNAL(clicked()), this, SLOT(cancelCropping()));
        QObject::connect(cropcancelBtn, SIGNAL(clicked()), this, SLOT(cancelCropping()));
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
        width = (image->p2.x() - image->p1.x() + 1)/image->scaleW;
        height = (image->p2.y() - image->p1.y() + 1)/image->scaleH;
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

// ResizeDialog object to get required image size
ResizeDialog:: ResizeDialog(QWidget *parent, int img_width, int img_height) : QDialog(parent)
{
    setupUi(this);
    frame->hide();
    resize(353, 200);
    QIntValidator validator(this);
    widthEdit->setValidator(&validator);
    heightEdit->setValidator(&validator);
    spinWidth->setValue(img_width*2.54/300);
    spinHeight->setValue(img_height*2.54/300);
    QObject::connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(toggleAdvanced(bool)));
    QObject::connect(spinWidth, SIGNAL(valueChanged(double)), this, SLOT(onValueChange(double)));
    QObject::connect(spinHeight, SIGNAL(valueChanged(double)), this, SLOT(onValueChange(double)));
    QObject::connect(spinDPI, SIGNAL(valueChanged(int)), this, SLOT(onValueChange(int)));
    widthEdit->setFocus();
}

void
ResizeDialog:: toggleAdvanced(bool checked)
{
    if (checked)
        frame->show();
    else
        frame->hide();
}

void
ResizeDialog:: onValueChange(int)
{
    int DPI = spinDPI->value();
    widthEdit->setText( QString::number(round(DPI * spinWidth->value()/2.54)));
    heightEdit->setText( QString::number(round(DPI * spinHeight->value()/2.54)));
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
    app.setOrganizationName("QmageView");
    app.setApplicationName("QmageView");
    Window *win = new Window();
    if (argc > 1) { // TODO : path existance check
        QString path = QString::fromUtf8(argv[1]);
        win->openImage(path);
    }
    else {
        QPixmap pm = QPixmap(":/images/nidhi.jpg");
        win->image->setImage(pm);
        win->adjustWindowSize();
    }
    win->show();
    return app.exec();
}
