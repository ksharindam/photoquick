#pragma once
#include <QImage>
#include <QSystemTrayIcon>
#include <QDebug>
#include "exif.h"

#define PROG_NAME       "PhotoQuick"
#define PROG_VERSION    "4.16.1"
#define COPYRIGHT_YEAR  "2017-2023"
#define AUTHOR_NAME     "Arindam Chaudhuri"
#define AUTHOR_EMAIL    "ksharindam@gmail.com"
#define PROJ_RELEASE    "https://github.com/ksharindam/photoquick/releases"


#define MIN(a,b) ({ __typeof__ (a) _a = (a); \
                    __typeof__ (b) _b = (b); \
                    _a < _b ? _a : _b; })

#define MAX(a,b) ({ __typeof__ (a) _a = (a); \
                    __typeof__ (b) _b = (b); \
                    _a > _b ? _a : _b; })

template<class T>
inline const T& min(const T& a, const T& b)
{
    return (b < a) ? b : a;
}

template<class T>
inline const T& max(const T& a, const T& b)
{
    return (b > a) ? b : a;
}

template<class T>
inline const T& clamp( const T& v, const T& lo, const T& hi )
{
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

#define SQR(x) ((x)*(x))


void fitToSize(int W, int H, int max_w, int max_h, int &out_w, int &out_h);

// fit inside the max size if larger than the size
void shrinkToFitSize(int W, int H, int max_w, int max_h, int &out_w, int &out_h);

// returns the scale of obj when fit inside max_w and max_h
float fitToSizeScale(float w, float h, float max_w, float max_h);

// round off a float upto given decimal point
float roundOff(float num, int dec);

// waits for the specified time in milliseconds
void waitFor(int millisec);

// get file format from magic numbers
const char* getFormat(QString filename);

// convert ARGB32 image to RGB32 image with color background
QImage setImageBackgroundColor(QImage img, QRgb color);

// load an image from file
// Returns an autorotated image according to exif data
QImage loadImage(QString filename);

// saves img as jpeg with that exif
bool saveJpegWithExif(QImage img, int quality, QString filename, ExifInfo &exif);

// get filesize in bytes when a QImage is saved as jpeg
int getJpgFileSize(QImage img, int quality=-1);


// creates a FILE* from QString filename
FILE* qfopen(QString filename, const char *mode);

// returns the path of the desktop directory
QString desktopPath();

class Notifier : public QSystemTrayIcon
{
public:
    Notifier(QObject *parent);
    void notify(QString title, QString message);
};

void debug(const char *format, ...);


// Runtime detection of byte order
inline bool isBigEndian()
{
    int i=1; return ! *((char *)&i);
}

// Compile time detection of byte order
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
enum
{
    CHANNEL_B,
    CHANNEL_G,
    CHANNEL_R,
    CHANNEL_A
};
#else
enum
{
    CHANNEL_A,
    CHANNEL_R,
    CHANNEL_G,
    CHANNEL_B
};
#endif


// check if armhf architecture, if not then it is assumed to be x86
#ifdef __arm__
#define ARCH "armhf"
//#elif __x86_64__
#else
#define ARCH "x86_64"
#endif
