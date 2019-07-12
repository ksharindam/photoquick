#pragma once
/* This file is a part of qmageview program, which is GPLv3 licensed */

#include <QImage>
#include <QSystemTrayIcon>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

// fit inside the max size if larger than the size
void fitToSize(int W, int H, int max_w, int max_h, int &out_w, int &out_h);

// round off a float upto given decimal point
float roundOff(float num, int dec);

// waits for the specified time in milliseconds
void waitFor(int millisec);

// load an image from file
// Returns an autorotated image according to exif data
QImage loadImage(QString filename);

class Notifier : public QSystemTrayIcon
{
public:
    Notifier(QObject *parent);
    void notify(QString title, QString message);
};
