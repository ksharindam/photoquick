/* This file is a part of photoquick program, which is GPLv3 licensed */

#include "common.h"
#include <QTimer>
#include <QEventLoop>
#include <QFile>
#include <QBuffer>
#include <QTransform>
#include <QIcon>
#include <QImageReader>
#include <QPainter>
#include <QDesktopServices>
#include <cmath>
#include <unistd.h> // dup()



void fitToSize(int W, int H, int max_w, int max_h, int &out_w, int &out_h)
{
    out_w = max_w;
    out_h = round((max_w/(float)W)*H);
    if (out_h > max_h) {
        out_h = max_h;
        out_w = round((max_h/(float)H)*W);
    }
}

// resize to fit if W and H is larger than max_w and max_h keeping aspect ratio
void shrinkToFitSize(int W, int H, int max_w, int max_h, int &out_w, int &out_h)
{
    if (W<=max_w and H<=max_h) {
        out_w = W;
        out_h = H;
        return;
    }
    fitToSize(W, H, max_w, max_h, out_w, out_h);
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

const char* getFormat(QString filename)
{
    FILE *f = qfopen(filename, "rb");
    if (f==NULL)
        return "";
    uchar buff[12]={};
    for (int i=0, c=0; i<12 && (c=getc(f))!=EOF; i++){
        buff[i] = c;
    }
    fclose(f);
    if (buff[0]==0xFF && buff[1]==0xD8 && buff[2]==0xFF)
        return "jpeg";
    if (buff[0]==0x89 && buff[1]=='P' && buff[2]=='N' && buff[3]=='G')
        return "png";
    if (buff[0]=='G' && buff[1]=='I' && buff[2]=='F' && buff[3]=='8')
        return "gif";
    if (buff[8]=='W' && buff[9]=='E' && buff[10]=='B' && buff[11]=='P')
        return "webp";
    return "";
}


QImage setImageBackgroundColor(QImage img, QRgb color)
{
    if (color==0x0)// keep transparency
        return img;
    QImage result(img.width(), img.height(), QImage::Format_RGB32);
    result.fill(color);
    QPainter painter(&result);
    painter.drawImage(0,0, img);
    painter.end();
    return result;
}


// load an image from file
QImage loadImage(QString fileName)
{
    QImage img(fileName);
    if (img.isNull()){
        // may be file extension is wrong, ignore extension and read again
        QImageReader reader(fileName);
        reader.setDecideFormatFromContent(true);
        reader.read(&img);
        if (img.isNull())
            return img;
    }
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

/* When we add exif ?
  Exif is added if either the passed Exif is not empty or resolution is > 1MP.
  If image is >1M, even if exif empty, we add exif to add thumbnail.
  If output resolution is <0.3MP, original exif info is discarded and only
  explicitly added exif infos are saved (such as, DPI, Software).
*/
bool saveJpegWithExif(QImage img, int quality, QString out_filename, ExifInfo &exif)
{
    if (exif.empty() && (img.width()*img.height()<1000000))
        return img.save(out_filename, "JPEG", quality);

    // fix Tag_Orientation
    if (exif.count(0x0112)>0) {
        exif[0x0112].integer = 1;
    }

    FILE *out = qfopen(out_filename, "w");
    if (!out) {
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
        thumb.save(&thumb_buff, "JPEG");
        ok = write_jpeg_with_exif(buff.buffer().data(), buff.size(),
                                thumb_buff.buffer().data(), thumb_buff.size(), exif, out);
        thumb_buff.buffer().clear();
    }
    else {
        ok = write_jpeg_with_exif(buff.buffer().data(), buff.size(), NULL, 0, exif, out);
    }
    fclose(out);
    buff.buffer().clear();
    return ok;
}


int getJpgFileSize(QImage image, int quality)
{
    if (image.isNull()) return 0;

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPG", quality);
    if (image.width()*image.height() >= 1000000) {
        QImage thumbnail = image.width()>image.height() ?
                            image.scaledToWidth(160) : image.scaledToHeight(160);
        thumbnail.save(&buffer, "JPG");
    }
    int filesize = buffer.size();
    buffer.buffer().clear();
    buffer.close();
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

// returns the path of the desktop directory
QString desktopPath()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
#else
    return QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
#endif
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
