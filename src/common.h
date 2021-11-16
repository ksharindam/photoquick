#pragma once
#include <QImage>
#include <QSystemTrayIcon>
#include <QDebug>

#define PROG_NAME       "PhotoQuick"
#define PROG_VERSION    "4.8.0"
#define COPYRIGHT_YEAR  "2017-2021"
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

// fit inside the max size if larger than the size
void fitToSize(int W, int H, int max_w, int max_h, int &out_w, int &out_h);

// returns the scale of obj when fit inside max_w and max_h
float fitToSizeScale(float w, float h, float max_w, float max_h);

// round off a float upto given decimal point
float roundOff(float num, int dec);

// waits for the specified time in milliseconds
void waitFor(int millisec);

// load an image from file
// Returns an autorotated image according to exif data
QImage loadImage(QString filename);

// Reads exif data from exif_filename and saves img as jpeg with that exif
bool saveJpegWithExif(QImage img, int quality, QString out_filename, QString exif_filename);

// get filesize in bytes when a QImage is saved as jpeg
int getJpgFileSize(QImage img, int quality=-1);


// creates a FILE* from QString filename
FILE* qfopen(QString filename, const char *mode);

class Notifier : public QSystemTrayIcon
{
public:
    Notifier(QObject *parent);
    void notify(QString title, QString message);
};

void debug(const char *format, ...);

// Byte order is ARGB if big endian else BGRA
inline bool isBigEndian()
{
    int i=1; return ! *((char *)&i);
}
