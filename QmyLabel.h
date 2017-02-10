#ifndef QMYLABEL_H
#define QMYLABEL_H

#include <QObject>
#include <QPoint>
#include <QLabel>
#include <QMouseEvent>

class QmyLabel : public QLabel
{
    Q_OBJECT
public :
    QmyLabel(QWidget *parent = 0) : QLabel(parent)
    {
        this->setMouseTracking(true);
    }
    ~ QmyLabel(){};
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
signals:
    void mouseMoved( QPoint );
    void mousePressed( QPoint );
    void mouseReleased( QPoint );
};


#endif