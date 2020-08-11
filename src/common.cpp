/* This file is a part of photoquick program, which is GPLv3 licensed */

#include "common.h"
#include <QTimer>
#include <QEventLoop>
#include <QFile>
#include <QBuffer>
#include <QTransform>
#include <QIcon>
#include <cmath>

// resize to fit if W and H is larger than max_w and max_h keeping aspect ratio
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
QImage loadImage(QString fileName)
{
    QImage img(fileName);
    if (img.isNull()) return img;
    // Converted because filters can only be applied to RGB32 or ARGB32 image
    if (img.hasAlphaChannel() && img.format()!=QImage::Format_ARGB32)
        img = img.convertToFormat(QImage::Format_ARGB32);
    else if (!img.hasAlphaChannel() and img.format()!=QImage::Format_RGB32)
        img = img.convertToFormat(QImage::Format_RGB32);
    // Get jpg orientation
    char *filename = fileName.toUtf8().data();
    FILE *f = fopen(filename, "rb");
    int orientation = getOrientation(f);
    fclose(f);
    // rotate if required
    QTransform transform;
    switch (orientation) {
        case 6:
            return img.transformed(transform.rotate(90));
        case 3:
            return img.transformed(transform.rotate(180));
        case 8:
            return img.transformed(transform.rotate(270));
    }
    return img;
}


int getJpgFileSize(QImage image, int quality)
{
    if (image.isNull()) return 0;

	QByteArray bArray;
	QBuffer buffer(&bArray);
	buffer.open(QIODevice::WriteOnly);
	image.save(&buffer, "JPG", quality);
	int filesize = bArray.size();
	bArray.clear();
	buffer.close();
    return filesize;
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


#ifdef DEBUG
void debug(const char *format, ...)
{
    va_list args ;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
#else
void debug(const char *, ...) {/* do nothing*/}
#endif
