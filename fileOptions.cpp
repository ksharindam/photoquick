// This file is used to open, save, next, prev, zoom images

#include "fileOptions.h"
#include <QFile>
#include "exif.h"
#include <QFileDialog>
#include <QPixmap>
#include <QDesktopWidget>
#include <QApplication>
#include <iostream>
#include <QTransform>
#include <QIODevice>
#include <QMovie>
//#include <cstdlib>
//    std::system("date +'%N'");

// For Debug Output...
//      std::cout << QString("Value : %1").arg(name).toStdString() << std::endl;


fileOptions::fileOptions(QMainWindow *mainWindow,QStatusBar *Statusbar, QLabel *label, QPixmap *pixmap, QPixmap *pixmap_scaled, qreal *Scale, QVBoxLayout *Vbox, QPushButton *slideshowBtn, int offset_X, int offset_Y)
{
    MainWindow = mainWindow;
    statusbar = Statusbar;
    Label = label;
    scale = Scale;
    pm_scaled = pixmap_scaled;
    pm = pixmap;
    vbox = Vbox;
    slideshowButton = slideshowBtn;
    offset_x = offset_X;
    offset_y = offset_Y;
    timer_created = false;
    slideshow_active = false;
}


void fileOptions::openImage ()
{
    QString fileOpen = QFileDialog::getOpenFileName(MainWindow,
                                      "Select Image to Open", fileName,
                                      "Image files (*.jpg *.png *.jpeg *.svg *.bmp *.gif *.xpm *.ppm)");
    if (!fileOpen.isEmpty()){
        openfile(fileOpen);
        fileName = fileOpen;
    }
}

void fileOptions::saveImage ()
{
    QString fileSave = QFileDialog::getSaveFileName(MainWindow,
                                      "Select Image to Save", fileName,
                                      "Image files (*.jpg *.jpeg *.png *.bmp *.xpm *.ppm)" );
    if (!fileSave.isEmpty()){
      int quality = -1;
      if ( !(fileSave.endsWith(".jpg",Qt::CaseInsensitive) or fileSave.endsWith(".jpeg",Qt::CaseInsensitive)))
        quality = -1;
      pm->save(fileSave,0,quality);
      fileName = fileSave;
    }
}

void fileOptions::openfile (QString filename)
{
    if (filename.endsWith(".gif", Qt::CaseInsensitive)){ // If image is animation
    QMovie *movie = new QMovie(filename);
    Label->setMovie(movie);
    movie->start();
    }
    else { // If image is not animation format
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    pm->loadFromData(file.readAll()); // after *pm = QPixmap(filename)
    file.close();
    int Orientation = 0;
//  
    file.open(QIODevice::ReadOnly);
    Exif *exif = new Exif();
    exif->getExifOrientation(file, &Orientation);
    file.close();

    if ( Orientation == 6){
       QTransform transform;
       QTransform trans = transform.rotate(90);
       *pm = pm->transformed(trans);}
    else if ( Orientation == 3){
       QTransform transform;
       QTransform trans = transform.rotate(180);
       *pm = pm->transformed(trans);}


    qreal imgwidth = pm->width();
    qreal imgheight = pm->height();
    int btnboxwidth = 2*(vbox->geometry().width());
    QDesktopWidget *desk = QApplication::desktop();
    int screen_width = desk->availableGeometry().width();
    int screen_height = desk->availableGeometry().height();
    qreal maxwidth = screen_width-(btnboxwidth+2*offset_x);
    qreal maxheight = screen_height-(offset_y+offset_x+4+32);  // 32 for statusbar height in crop mode
    // When image does not fit screen
    if (imgwidth>maxwidth || imgheight>maxheight){  
        if (maxwidth/maxheight > imgwidth/imgheight){
          *pm_scaled = pm->scaledToHeight(maxheight, Qt::SmoothTransformation);
          *scale = maxheight/imgheight;
        }
        else {
          *pm_scaled = pm->scaledToWidth(maxwidth, Qt::SmoothTransformation);
          *scale = maxwidth/imgwidth;
        }
    }
    else {  // When image is smaller than screen size
        *pm_scaled = pm->copy();
        *scale = 1.0;
    }
    MainWindow->resize(pm_scaled->width()+btnboxwidth+4, pm_scaled->height()+4+32); // 32 for statusbar
    MainWindow->move((screen_width - (MainWindow->width()+2*offset_x))/2, (screen_height - (MainWindow->height()+offset_y+offset_x))/2);
    Label->setPixmap(*pm_scaled);
    statusbar->showMessage(QString("Resolution : %1x%2 , Scale : %3x").arg(int(imgwidth)).arg(int(imgheight)).arg(float(*scale)));}
    fileName = filename;
    QFileInfo fi(fileName);
    QString name = fi.fileName();
    MainWindow->setWindowTitle(name);
}

void fileOptions::openNext()
{
    QFileInfo fi(fileName);
    QString basedir = fi.absolutePath();
    QString filename = fi.fileName();
    QStringList filter;
    filter << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.svg" << "*.bmp" << "*.tiff";
    QStringList imagelist = fi.dir().entryList(filter);

    int index = imagelist.indexOf(filename);
    if (index == imagelist.count()-1)
      index = -1;
    QString nextfile = imagelist.takeAt(index+1);
    openfile(basedir.append("/").append(nextfile));
}

void fileOptions::openPrev()
{
    QFileInfo fi(fileName);
    QString basedir = fi.absolutePath();
    QString filename = fi.fileName();
    QStringList filter;
    filter << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.svg" <<"*.bmp" << "*.tiff";
    QStringList imagelist = fi.dir().entryList(filter);

    int index = imagelist.indexOf(filename);
    if (index != 0){
      QString prevfile = imagelist.takeAt(index-1);
      openfile(basedir.append("/").append(prevfile));}
}

void fileOptions::toggleSlideshow()
{
  if (! slideshow_active){
    if (!timer_created ){
      timer = new QTimer(this);
      QObject::connect(timer, SIGNAL(timeout()), this, SLOT(openNext()));
      timer_created = true;}
    timer->start(3000);
    slideshow_active = true;
    slideshowButton->setIcon(QIcon(":/pause.png"));
    openNext();
  }
  else {
    timer->stop();
    slideshow_active = false;
    slideshowButton->setIcon(QIcon(":/play.png"));
  }
}

void fileOptions::zoomIn()
{
    *scale *= 6.0/5.0;
    *pm_scaled = pm->scaledToHeight(*scale*pm->height(), Qt::SmoothTransformation);
    Label->setPixmap( *pm_scaled);
    statusbar->showMessage(QString("Resolution : %1x%2 , Scale : %3x").arg(int(pm->width())).arg(int(pm->height())).arg(float(*scale)));
} 

void fileOptions::zoomOut()
{
    *scale *= 5.0/6.0;
    *pm_scaled = pm->scaledToHeight(*scale*pm->height(), Qt::SmoothTransformation);
    Label->setPixmap( *pm_scaled);
    statusbar->showMessage(QString("Resolution : %1x%2 , Scale : %3x").arg(int(pm->width())).arg(int(pm->height())).arg(float(*scale)));
} 

void fileOptions::originalSize()
{
    *scale = 1.0;
    *pm_scaled = pm->copy();
    Label->setPixmap( *pm_scaled);
    statusbar->showMessage(QString("Resolution : %1x%2 , Scale : 1x").arg(int(pm->width())).arg(int(pm->height())));
} 
