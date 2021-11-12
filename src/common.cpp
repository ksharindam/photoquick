/* This file is a part of photoquick program, which is GPLv3 licensed */

#include "common.h"
#include "exif.h"
#include <QTimer>
#include <QEventLoop>
#include <QFile>
#include <QBuffer>
#include <QTransform>
#include <QIcon>
#include <cmath>
#include <unistd.h> // dup()


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

float fitToSizeScale(float w, float h, float max_w, float max_h)
{
    if (max_w/max_h > w/h) {//e.g- when max -18:9, img- 16:9 fit top
        return max_h/h;
    }
    else
        return max_w/w;
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
    FILE *f = qfopen(fileName, "rb");
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


bool saveJpegWithExif(QImage img, int quality, QString out_filename, QString exif_filename)
{
    // image too small, do not add thumbnail
    if (img.width()*img.height()<300000)
        return img.save(out_filename, "JPEG", quality);

    FILE *infile = qfopen(exif_filename, "r");
    if (!infile)
        return img.save(out_filename, "JPEG", quality);
    ExifInfo exif;
    exif_read(exif, infile);
    fclose(infile);
    // if image is >1M, even if exif empty, we add exif to add thumbnail
    if (exif.empty() && (img.width()*img.height()<1000000))
        return img.save(out_filename, "JPEG", quality);

    FILE *out = qfopen(out_filename, "w");
    if (!out) {
        exif_free(exif);
        return false;
    }
    QBuffer buff;
    buff.open(QIODevice::WriteOnly);
    img.save(&buff, "JPEG", quality);
    bool ok;
    if (img.width()*img.height()>=1000000){// add a thumbnail
        QBuffer thumb_buff;
        thumb_buff.open(QIODevice::WriteOnly);
        // recommended thumbnail resolution is 160x120
        QImage thumb = img.width()>img.height() ? img.scaledToWidth(160) : img.scaledToHeight(160);
        thumb.save(&thumb_buff, "JPEG", -1);
        ok = write_jpeg_with_exif(buff.buffer().data(), buff.size(),
                                thumb_buff.buffer().data(), thumb_buff.size(), exif, out);
        thumb_buff.buffer().clear();
    }
    else {
        ok = write_jpeg_with_exif(buff.buffer().data(), buff.size(), NULL, 0, exif, out);
    }
    fclose(out);
    buff.buffer().clear();
    exif_free(exif);
    return ok;
}


int getJpgFileSize(QImage image, int quality)
{
    if (image.isNull()) return 0;

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPG", quality);
    buffer.close();
    int filesize = buffer.size();
    buffer.buffer().clear();
    return filesize;
}


/* On linux we can simply do,
    char *filename = fileName.toUtf8().data();
    FILE *f = fopen(filename, "rb");
   But on Windows, this fails to open unicode filenames.
   The function below solves this issue.
*/
// Creates a FILE* from QString filename
FILE* qfopen(QString filename, const char *mode)
{
    QIODevice::OpenMode io_mode = QIODevice::NotOpen;
    if (QString(mode).contains("r"))
        io_mode |= QIODevice::ReadOnly;
    if (QString(mode).contains("w"))
        io_mode |= QIODevice::WriteOnly;
    if (QString(mode).contains("a"))
        io_mode |= QIODevice::Append;
    if (QString(mode).contains("r+") or QString(mode).contains("w+"))
        io_mode |= QIODevice::ReadWrite;

    QFile qf(filename);
    if (!qf.open(io_mode))
        return NULL;
    int fd = dup(qf.handle());
    qf.close();
    FILE *f = fdopen(fd, mode);
    return f;
}


Notifier:: Notifier(QObject *parent) : QSystemTrayIcon(parent)
{
    setIcon(QIcon(":/icons/photoquick.png"));
}

void
Notifier:: notify(QString title, QString message)
{
    show();
    waitFor(200);// otherwise notification popups in wrong position
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
