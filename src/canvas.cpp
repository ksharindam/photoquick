/* This file is a part of photoquick program, which is GPLv3 licensed */

#include "canvas.h"
#include <QDebug>
#include <QSizePolicy>
#include <QTransform>
#include <QPainter>
#include <cmath>


Canvas:: Canvas(QWidget *parent, QScrollArea *scrollArea, ImageData *img_dat) : QLabel(parent)
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
    if (data->animation)
        movie()->deleteLater();
    else
        data->image = QImage();
    data->animation = true;
    setMovie(anim);
    anim->start();
}

void
Canvas:: setImage(QImage img)
{
    if (data->animation) {
        data->animation = false;
        movie()->deleteLater();
    }
    data->image = img;
    showScaled();
}

void
Canvas:: showScaled()
{
    QPixmap pm = QPixmap::fromImage(data->image);
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
