/*
This file is a part of qmageview program, which is GPLv3 licensed
*/
#include "image.h"
#include <QSizePolicy>
#include <QTransform>
#include <QPainter>
#include <QRect>
#include <cmath>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


Image:: Image(QWidget *parent, QScrollArea *scrollArea) : QLabel(parent)
{
    vScrollbar = scrollArea->verticalScrollBar();
    hScrollbar = scrollArea->horizontalScrollBar();
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
    mouse_pressed = false;
    animation = false;
    crop_mode = false;
    scale = 1.0;
}

void
Image:: setAnimation(QMovie *anim)
{
    scale = 1.0;
    pic = QPixmap();
    setMovie(anim);
    anim->start();
    animation = true;
}

void
Image:: setImage(QPixmap pix)
{
    pic = pix;                // Save original pixmap
    showScaled();
    animation = false;
}

void
Image:: showScaled()
{
    QPixmap pm;
    if (scale == 1.0)
        pm = pic.copy();
    else
        pm = pic.scaledToHeight(scale*pic.height(), Qt::SmoothTransformation);
    setPixmap(pm);
    emit imageUpdated();
}

void
Image:: rotate(int degree)
{
    QTransform transform;
    transform.rotate(degree);
    pic = pic.transformed(transform);
    showScaled();
}

void
Image:: zoomBy(float factor)
{
    scale *= factor;
    showScaled();
}

void
Image:: enableCropMode(bool enable)
{
    if (enable) {
        crop_mode = true;
        // scaleW & scaleH give better accuracy while cropping downscaled image
        scaleW = float(pixmap()->width())/pic.width();
        scaleH = float(pixmap()->height())/pic.height();
        pm_tmp = pixmap()->copy();
        topleft = QPoint(0,0);
        btmright = QPoint(pixmap()->width()-1, pixmap()->height()-1);
        last_pt = QPoint(btmright);
        p1 = QPoint(topleft);
        p2 = QPoint(btmright);
        crop_width = 3.5;
        crop_height = 4.5;
        lock_crop_ratio = false;
        imgAspect = float(pic.width())/pic.height();
        drawCropBox();
    }
    else {
        crop_mode = false;
        showScaled();
    }
}

void
Image:: mousePressEvent(QMouseEvent *ev)
{
    clk_pos = ev->pos();
    clk_global = ev->globalPos();
    v_scrollbar_pos = vScrollbar->value();
    h_scrollbar_pos = hScrollbar->value();
    mouse_pressed = true;
    if (not crop_mode) return;
    p1 = QPoint(topleft); // Save initial pos of cropbox
    p2 = QPoint(btmright);
    // Determine which position is clicked
    if (QRect(topleft, QSize(60, 60)).contains(clk_pos)) // Topleft is clicked
        clk_area = 1;
    else if (QRect(btmright, QSize(-60, -60)).contains(clk_pos)) // Bottom right is clicked
        clk_area = 2;
    else if (QRect(topleft, btmright).contains(clk_pos)) // clicked inside cropbox
        clk_area = 3;
    else
        clk_area = 0;
}

void
Image:: mouseReleaseEvent(QMouseEvent *)
{
    mouse_pressed = false;
    if (not crop_mode) return;
    topleft = p1;
    btmright = p2;
}

void
Image:: mouseMoveEvent(QMouseEvent *ev)
{
    if (not mouse_pressed) return;
    if (not crop_mode) {
        // Handle click and drag to scroll
        vScrollbar->setValue(v_scrollbar_pos + clk_global.y() - ev->globalY());
        hScrollbar->setValue(h_scrollbar_pos + clk_global.x() - ev->globalX());
        return;
    }
    // Handle crop mode
    QPoint moved = ev->pos() - clk_pos;
    float boxAspect = crop_width/crop_height;

    if (clk_area == 1) { // Top left corner is clicked
        QPoint new_p1 = topleft + moved;
        p1 = QPoint(MAX(0, new_p1.x()), MAX(0, new_p1.y()));
        if (lock_crop_ratio) {
            if (imgAspect>boxAspect) p1.setX(round(p2.x() - (p2.y()-p1.y()+1)*boxAspect -1));
            else p1.setY(round(p2.y() - (p2.x()-p1.x()+1)/boxAspect -1));
        }
    }
    else if (clk_area == 2) { // Bottom right corner is clicked
        QPoint new_p2 = btmright + moved;
        p2 = QPoint(MIN(last_pt.x(), new_p2.x()), MIN(last_pt.y(), new_p2.y()));
        if (lock_crop_ratio) {
            if (imgAspect>boxAspect) p2.setX(round(p1.x() + (p2.y()-p1.y()+1)*boxAspect -1));
            else p2.setY(round(p1.y() + (p2.x()-p1.x()+1)/boxAspect -1));
        }
    }
    else if (clk_area == 3) { // clicked inside cropbox but none of the corner selected.
        int min_dx, max_dx, min_dy, max_dy, dx, dy;
        min_dx = -topleft.x();
        max_dx = last_pt.x()-btmright.x();
        min_dy = -topleft.y();
        max_dy = last_pt.y()-btmright.y();
        dx = (moved.x() < 0) ? MAX(moved.x(), min_dx) : MIN(moved.x(), max_dx);
        dy = (moved.y() < 0) ? MAX(moved.y(), min_dy) : MIN(moved.y(), max_dy);
        p1 = topleft + QPoint(dx, dy);
        p2 = btmright + QPoint(dx, dy);
    }

    drawCropBox();
}

void
Image:: drawCropBox()
{
    QPixmap pm = pm_tmp.copy();
    QPixmap pm_box = pm.copy(p1.x(), p1.y(), p2.x()-p1.x(), p2.y()-p1.y());
    QPainter painter(&pm);
    painter.fillRect(0,0, pm.width(), pm.height(), QColor(127,127,127,127));
    painter.drawPixmap(p1.x(), p1.y(), pm_box);
    painter.drawRect(p1.x(), p1.y(), p2.x()-p1.x(), p2.y()-p1.y());
    painter.drawRect(p1.x(), p1.y(), 59, 59);
    painter.drawRect(p2.x(), p2.y(), -59, -59);
    painter.setPen(Qt::white);
    painter.drawRect(p1.x()+1, p1.y()+1, 57, 57);
    painter.drawRect(p2.x()-1, p2.y()-1, -57, -57);
    painter.end();
    setPixmap(pm);
    emit imageUpdated();
}

void
Image:: lockCropRatio(bool checked)
{
    lock_crop_ratio = checked;
}

void
Image:: setCropWidth(double value)
{
    crop_width = float(value);
}

void
Image:: setCropHeight(double value)
{
    crop_height = float(value);
}

void
Image:: cropImage()
{
    int w, h;
    w = round((btmright.x()-topleft.x()+1)/scaleW);
    h = round((btmright.y()-topleft.y()+1)/scaleH);
    QPixmap pm = pic.copy(round(topleft.x()/scaleW), round(topleft.y()/scaleH), w, h);
    setImage(pm);
}
