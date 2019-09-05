/* This file is a part of qmageview program, which is GPLv3 licensed */

#include "common.h"
#include "exif.h"
#include <QTimer>
#include <QEventLoop>
#include <QFile>
#include <QTransform>
#include <QIcon>
#include <cmath>

void fitToSize(int W, int H, int max_w, int max_h, int &out_w, int &out_h)
{
    if (W<=max_w and H<=max_h) {
        out_w = W;
        out_h = H;
        return;
    }
    out_w = max_w;
    out_h = round((max_w/(float)W)*H);
    if (out_h > max_h) {
        out_h = max_h;
        out_w = round((max_h/(float)H)*W);
    }
}

// round off a float upto given decimal point
float roundOff(float num, int dec)
{
    double m = (num < 0.0) ? -1.0 : 1.0;   // check if input is negative
    double pwr = pow(10, dec);
    return float(floor((double)num * m * pwr + 0.5) / pwr) * m;
}

// waits for the specified time in milliseconds
void waitFor(int millisec)
{
    // Creates an eventloop to wait for a time
    QEventLoop *loop = new QEventLoop();
    QTimer::singleShot(millisec, loop, SLOT(quit()));
    loop->exec();
    loop->deleteLater();
}

// load an image from file
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

Notifier:: Notifier(QObject *parent): QSystemTrayIcon(QIcon(":/images/image.png"), parent)
{
}

void
Notifier:: notify(QString title, QString message)
{
    show();
    waitFor(200);
    showMessage(title, message);
    QTimer::singleShot(3000, this, SLOT(deleteLater()));
}
