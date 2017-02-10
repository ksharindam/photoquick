#include "QmyLabel.h"

void QmyLabel::mouseMoveEvent(QMouseEvent *event)
{
    QPoint p(event->x(), event->y());
    emit mouseMoved(p);
}

void QmyLabel::mousePressEvent(QMouseEvent *event)
{
    QPoint p(event->x(), event->y());
    emit mousePressed(p);
}

void QmyLabel::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint p(event->x(), event->y());
    emit mouseReleased(p);
}

