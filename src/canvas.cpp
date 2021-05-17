/* This file is a part of photoquick program, which is GPLv3 licensed */

#include "canvas.h"
#include <QDebug>
#include <QSizePolicy>
#include <QTransform>
#include <QPainter>
#include <cmath>


Canvas:: Canvas(QScrollArea *scrollArea, ImageData *img_dat) : QLabel(scrollArea)
{
    vScrollbar = scrollArea->verticalScrollBar();
    hScrollbar = scrollArea->horizontalScrollBar();
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
    data = img_dat;
    mouse_pressed = false;
    drag_to_scroll = true;
    scale = 1.0;
}

void
Canvas:: setAnimation(QMovie *anim)
{
    scale = 1.0;
    if (this->animation)
        movie()->deleteLater();
    else
        data->image = QImage();
    this->animation = true;
    setMovie(anim);
    anim->start();
}

void
Canvas:: setImage(QImage img)
{
    if (this->animation) {
        this->animation = false;
        movie()->deleteLater();
    }
    data->image = img;
    showScaled();
}

void
Canvas:: setMask(QImage mask_img)
{
    tmp_image = data->image;
    // using 1 bit per pixel image as mask, reduces memory usage significantly
    mask = QImage(mask_img.width(), mask_img.height(), QImage::Format_MonoLSB);

    for (int y=0; y<mask_img.height(); y++)
    {
        uchar *row = mask.scanLine(y);
        QRgb* imgRow = (QRgb*) mask_img.constScanLine(y);
        for (int x=0; x<mask_img.width(); x++) {
            int val = qRed(imgRow[x]) > 127 ? 1 : 0;
            int bitmask = 1 << (x%8);
            row[x/8] = (row[x/8] & ~bitmask) | (val << (x%8));
        }
    }
    showScaled();
}

void
Canvas:: clearMask()
{
    mask = QImage();
    tmp_image = QImage();
    showScaled();
}

void
Canvas:: invertMask()
{
    mask.invertPixels();
    tmp_image = data->image;// prevents restoring of previously masked areas
    showScaled();
}

void
Canvas:: showScaled()
{
    QPixmap pm;
    if (not mask.isNull()) {
        // restore masked areas in data->image from tmp_image
        QImage pmImg = data->image.copy();

        for (int y=0, h=mask.height(); y<h; y++)
        {
            QRgb *row = (QRgb*)data->image.scanLine(y);
            QRgb *tmpRow = (QRgb*)tmp_image.constScanLine(y);
            QRgb *pmRow = (QRgb*) pmImg.scanLine(y);
            const uchar *maskRow = mask.constScanLine(y);
            for (int x=0, w=mask.width(); x<w; x++) {
                uchar masked = (maskRow[x/8] >> (x%8) ) & 0x01;
                if (masked == 1){
                    row[x] = tmpRow[x];
                    // add a green tone over masked region
                    pmRow[x] = qRgb(0.5*qRed(row[x]), 127+0.5*qGreen(row[x]), 0.5*qBlue(row[x]));
                }
            }
        }
        pm = QPixmap::fromImage(pmImg);
    }
    else {
        pm = QPixmap::fromImage(data->image);
    }
    if (scale != 1.0) {
        Qt::TransformationMode mode = floorf(scale) == ceilf(scale)? // integer scale
                                    Qt::FastTransformation : Qt::SmoothTransformation;
        pm = pm.scaledToHeight(scale*pm.height(), mode);
    }
    setPixmap(pm);
    emit imageUpdated();
}

void
Canvas:: rotate(int degree, Qt::Axis axis)
{
    QTransform transform;
    transform.rotate(degree, axis);
    data->image = data->image.transformed(transform);
    showScaled();
}

void
Canvas:: mousePressEvent(QMouseEvent *ev)
{
    clk_global = ev->globalPos();
    v_scrollbar_pos = vScrollbar->value();
    h_scrollbar_pos = hScrollbar->value();
    mouse_pressed = true;
    emit mousePressed(ev->pos());
}

void
Canvas:: mouseReleaseEvent(QMouseEvent *ev)
{
    mouse_pressed = false;
    emit mouseReleased(ev->pos());
}

void
Canvas:: mouseMoveEvent(QMouseEvent *ev)
{
    emit mouseMoved(ev->pos());
    if (not (mouse_pressed and drag_to_scroll)) return;
    // Handle click and drag to scroll
    vScrollbar->setValue(v_scrollbar_pos + clk_global.y() - ev->globalY());
    hScrollbar->setValue(h_scrollbar_pos + clk_global.x() - ev->globalX());
}



// ******************* Paint Canvas ******************
PaintCanvas:: PaintCanvas(QWidget *parent) : QLabel(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
}

void
PaintCanvas:: mousePressEvent(QMouseEvent *ev)
{
    emit mousePressed(ev->pos());
}

void
PaintCanvas:: mouseMoveEvent(QMouseEvent *ev)
{
    emit mouseMoved(ev->pos());
}

void
PaintCanvas:: mouseReleaseEvent(QMouseEvent *ev)
{
    emit mouseReleased(ev->pos());
}

void
PaintCanvas:: setImage(QImage img)
{
    setPixmap(QPixmap::fromImage(img));
}


QCursor roundCursor(int width)
{
    QPixmap pm(width, width);
    pm.fill(QColor(0,0,0,0));
    QPainter painter(&pm);
    painter.drawEllipse(0,0, width-1, width-1);
    painter.setPen(Qt::white);
    painter.drawEllipse(1,1, width-3, width-3);
    painter.end();
    return QCursor(pm);
}
