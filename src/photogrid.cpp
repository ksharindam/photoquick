/*
This file is a part of qmageview program, which is GPLv3 licensed
*/
#include "photogrid.h"
#include "exif.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPen>
#include <QDesktopWidget>
#include <QSettings>
//#include <QDebug>

GridDialog:: GridDialog(QImage img, QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    resize(1020, 640);
    QHBoxLayout *layout = new QHBoxLayout(scrollAreaWidgetContents);
    layout->setContentsMargins(0, 0, 0, 0);
    gridPaper = new GridPaper(this);
    layout->addWidget(gridPaper);
    thumbnailGr = new ThumbnailGroup(this);
    Thumbnail *thumbnail = new Thumbnail(img, frame);
    verticalLayout->addWidget(thumbnail);
    thumbnail->select(true);
    QObject::connect(thumbnail, SIGNAL(clicked(QImage)), gridPaper, SLOT(setPhoto(QImage)));
    thumbnailGr->append(thumbnail);
    QObject::connect(configureBtn, SIGNAL(clicked()), this, SLOT(configure()));
    QObject::connect(addPhotoBtn, SIGNAL(clicked()), this, SLOT(addPhoto()));
    QObject::connect(checkAddBorder, SIGNAL(clicked(bool)), gridPaper, SLOT(toggleBorder(bool)));
    QObject::connect(helpBtn, SIGNAL(clicked()), this, SLOT(showHelp()));
    gridPaper->photo = img;
}

void
GridDialog:: configure()
{
    GridSetupDialog *dialog = new GridSetupDialog(this);
    if ( dialog->exec()==QDialog::Accepted ) {
        gridPaper->paperW = dialog->paperW;
        gridPaper->paperH = dialog->paperH;
        gridPaper->rows = dialog->rows;
        gridPaper->cols = dialog->cols;
        gridPaper->W = dialog->W;
        gridPaper->H = dialog->H;
        gridPaper->DPI = dialog->DPI;
        gridPaper->setupGrid();
    }
}

void
GridDialog:: addPhoto()
{
    QString filefilter = "JPEG Images (*.jpg *jpeg);;PNG Images (*.png);;All Files (*)";
    QString filepath = QFileDialog::getOpenFileName(this, "Open Image", "", filefilter);
    if (filepath.isEmpty()) return;
    QImage img = loadImage(filepath);
    if (not img.isNull()) {
        Thumbnail *thumbnail = new Thumbnail(img, frame);
        verticalLayout->addWidget(thumbnail);
        QObject::connect(thumbnail, SIGNAL(clicked(QImage)), gridPaper, SLOT(setPhoto(QImage)));
        thumbnailGr->append(thumbnail);
    }
}

void
GridDialog:: accept()
{
    // Create final grid when ok is clicked
    gridPaper->createFinalGrid();
    QDialog::accept();
}

void
GridDialog:: showHelp()
{
    QString helptext = "Click on a image thumbnail to select an image to drop. Then click on the blank boxes to drop the selected photo.\n\n"
                       "If you want to create grid with more different photos then load photo by clicking Add Photo button.\n\n"
                       "You can change the photo of a box by selecting another image and clicking over the box.";
    QMessageBox::about(this, "How to Create Grid", helptext);
}


Thumbnail:: Thumbnail(QImage img, QWidget *parent) : QLabel(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    photo = img;
    setPixmap(QPixmap::fromImage(img.scaledToWidth(100)));
}

void
Thumbnail:: mousePressEvent(QMouseEvent *)
{
    emit clicked(photo);
}

void
Thumbnail:: select(bool selected)
{
    if (selected) {
        QImage img = photo.scaledToWidth(100);
        QPainter painter(&img);
        QPen pen(Qt::blue);
        pen.setWidth(4);
        painter.setPen(pen);
        painter.drawRect(2, 2 , 100-4, img.height()-4);
        painter.end();
        setPixmap(QPixmap::fromImage(img));
    }
    else
        setPixmap(QPixmap::fromImage(photo.scaledToWidth(100)));
}


ThumbnailGroup:: ThumbnailGroup(QObject *parent) : QObject(parent)
{
}

void
ThumbnailGroup:: append(Thumbnail *thumbnail)
{
    thumbnails << thumbnail;
    QObject::connect(thumbnail, SIGNAL(clicked(QImage)), this, SLOT(selectThumbnail(QImage)));
}

void
ThumbnailGroup:: selectThumbnail(QImage)
{
    for (int i=0;i<thumbnails.count();++i) {
        thumbnails[i]->select(false);
    }
    qobject_cast<Thumbnail*>(sender())->select(true);
}

// GridPaper class methods
GridPaper:: GridPaper(QWidget *parent) : QLabel(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
    add_border = true;
    QSettings settings(this);
    DPI = settings.value("DPI", 300).toInt();
    paperW = settings.value("PaperWidth", 1800).toInt();
    paperH = settings.value("PaperHeight", 1200).toInt();
    W = settings.value("ImageWidth", 413).toInt();
    H = settings.value("ImageHeight", 531).toInt();
    cols = settings.value("Cols", 4).toInt();
    rows = settings.value("Rows", 2).toInt();           // total no. of columns and rows
    setupGrid();
}

void
GridPaper:: setupGrid()
{
    boxes.clear();
    spacingX = (paperW-cols*W)/float(cols+1);
    spacingY = (paperH-rows*H)/float(rows+1);
    // Setup Foreground Grid
    float screenDPI = QApplication::desktop()->logicalDpiX();
    scale = screenDPI/DPI;
    float w = W*scale;
    float h = H*scale;
    float spacing_x = spacingX*scale;
    float spacing_y = spacingY*scale;
    for (int i=0; i<cols*rows; ++i) {
        int row = i/cols;            // Position of the box as row & col
        int col = i%cols;
        QRect box = QRect(spacing_x+col*(spacing_x+w), spacing_y+row*(spacing_y+h), w-1, h-1);
        boxes << box;
    }
    QPixmap pm = QPixmap(paperW*scale, paperH*scale);
    pm.fill();
    QPainter painter(&pm);
    foreach (QRect box, boxes)
        painter.drawRect(box);
    painter.end();
    setPixmap(pm);
}

void
GridPaper:: setPhoto(QImage img)
{
    photo = img;
}

void
GridPaper:: toggleBorder(bool ok)
{
    add_border = ok;
    QPixmap grid = *(pixmap());
    QPainter painter(&grid);
    foreach (int index, image_dict.keys()) {
        QPoint topleft = boxes[index].topLeft();
        QImage img = image_dict.value(index).scaled(W*scale, H*scale, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawImage(topleft, img);
        if (ok) painter.drawRect(topleft.x(), topleft.y(), img.width()-1, img.height()-1);
    }
    painter.end();
    setPixmap(grid);
}

void
GridPaper:: mouseMoveEvent(QMouseEvent *ev)
{
    // Change cursor whenever cursor comes over a box
    foreach (QRect box, boxes) {
        if (box.contains(ev->pos())) {
            setCursor(Qt::PointingHandCursor);
            return;
        }
    }
    setCursor(Qt::ArrowCursor);
}

void
GridPaper:: mousePressEvent(QMouseEvent *ev)
{
    foreach (QRect box, boxes) {
        if (box.contains(ev->pos())) {
            QPoint topleft = box.topLeft();
            QImage img = photo.scaled(W*scale, H*scale, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QPixmap bg = *(pixmap());
            QPainter painter(&bg);
            painter.fillRect(topleft.x(), topleft.y(), W*scale, H*scale, Qt::white);
            painter.drawImage(topleft, img);
            if (add_border)
                painter.drawRect(topleft.x(), topleft.y(), img.width()-1, img.height()-1);
            painter.end();
            setPixmap(bg);
            image_dict[boxes.indexOf(box)] = photo;
            break;
        }
    }
}

void
GridPaper:: createFinalGrid()
{
    photo_grid = QImage(paperW, paperH, QImage::Format_ARGB32);
    photo_grid.fill(Qt::white);
    QPainter painter(&photo_grid);
    foreach (int index, image_dict.keys()) {
        int row = index/cols;
        int col = index%cols;
        QPoint topleft = QPoint(spacingX+col*(spacingX+W), spacingY+row*(spacingY+H));
        QImage img = image_dict.value(index).scaled(W, H, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawImage(topleft, img);
        if (add_border)
            painter.drawRect(topleft.x(), topleft.y(), img.width()-1, img.height()-1);
    }
    painter.end();
}

// GridSetupDialog class functions
GridSetupDialog:: GridSetupDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
}

void
GridSetupDialog:: accept()
{
    QList<float> units;
    units << 1 << 1/2.54 << 1/25.4 ;
    DPI = spinDPI->value();
    float unit_mult = units[paperSizeUnit->currentIndex()];
    int paper_w = spinPaperWidth->value()*unit_mult*DPI;
    int paper_h = spinPaperHeight->value()*unit_mult*DPI;
    W = spinPhotoWidth->value()*DPI/2.54;
    H = spinPhotoHeight->value()*DPI/2.54;
    int rows1 = paper_h/H;
    int cols1 = paper_w/W;
    int rows2 = paper_w/H;
    int cols2 = paper_h/W;
    if (rows1*cols1 >= rows2*cols2) {
        paperW = paper_w;
        paperH = paper_h;
        rows = rows1;
        cols = cols1;
    }
    else {
        paperW = paper_h;
        paperH = paper_w;
        rows = rows2;
        cols = cols2;
    }
    QSettings settings(this);
    settings.setValue("DPI", DPI);
    settings.setValue("PaperWidth", paperW);
    settings.setValue("PaperHeight", paperH);
    settings.setValue("ImageWidth", W);
    settings.setValue("ImageHeight", H);
    settings.setValue("Rows", rows);
    settings.setValue("Cols", cols);
    QDialog::accept();
}

// Static functions
QImage loadImage(QString filename)
{
    QImage img(filename);
    if (img.isNull()) return img;
    // Get image orientation
    int size, orientation = 0;
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    size = file.size() < 65536 ? file.size() : 65536;  // load maximum of 64 kB
    char* buffer = new char[size];
    file.read(buffer, size);
    easyexif::EXIFInfo info;
    info.parseFrom((uchar*)buffer, size);
    orientation = info.Orientation;
    file.close();
    delete buffer;
    // rotate if required
    QTransform transform;
    switch (orientation) {
        case 6:
            return img.transformed(transform.rotate(90));
        case 3:
            return img.transformed(transform.rotate(180));
        case 8:
            return img.transformed(transform.rotate(270));
        default:
            return img;
    }
}

