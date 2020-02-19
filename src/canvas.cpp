/*
This file is a part of photoquick program, which is GPLv3 licensed
*/
#include "canvas.h"
#include <QDebug>
#include <QSizePolicy>
#include <QTransform>
#include <cmath>


Canvas:: Canvas(QWidget *parent, QScrollArea *scrollArea) : QLabel(parent)
{
    vScrollbar = scrollArea->verticalScrollBar();
    hScrollbar = scrollArea->horizontalScrollBar();
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
    mouse_pressed = false;
    animation = false;
    drag_to_scroll = true;
    scale = 1.0;
}

void
Canvas:: setAnimation(QMovie *anim)
{
    scale = 1.0;
    if (animation)
        movie()->deleteLater();
    else
        image = QImage();
    animation = true;
    setMovie(anim);
    anim->start();
}

void
Canvas:: setImage(QImage img)
{
    if (animation) {
        animation = false;
        movie()->deleteLater();
    }
    image = img;
    showScaled();
}

void
Canvas:: showScaled()
{
    QPixmap pm = QPixmap::fromImage(image);
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
    image = image.transformed(transform);
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
