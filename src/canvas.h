#pragma once
/*
Image Label Object to display the image.
*/

#include <QLabel>
#include <QMovie>
#include <QMouseEvent>
#include <QScrollArea>
#include <QScrollBar>


//This is the widget responsible for displaying image
class Canvas : public QLabel
{
    Q_OBJECT
public:
    Canvas(QWidget *parent, QScrollArea *scrollArea);
    void setAnimation(QMovie *anim);
    void setImage(QImage img);
    void showScaled();
    void rotate(int degree, Qt::Axis axis=Qt::ZAxis);
    // Variables
    QImage image;
    bool animation;
    float scale;
    bool drag_to_scroll;    // if click and drag moves image
private:
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    // Variables
    bool mouse_pressed;
    int v_scrollbar_pos, h_scrollbar_pos;
    QPoint clk_global;
    QScrollBar *vScrollbar, *hScrollbar;
signals:
    void mousePressed(QPoint);
    void mouseReleased(QPoint);
    void mouseMoved(QPoint);
    void imageUpdated();
};
