/*
This file is a part of qmageview program, which is GPLv3 licensed
*/
#ifndef PHOTOGRID_H
#define PHOTOGRID_H

#include "photogrid_dialog.h"
#include <QLabel>
#include <QMouseEvent>

QT_BEGIN_NAMESPACE
// Thumbnail class holds the pixmap of a photo
class Thumbnail : public QLabel 
{
    Q_OBJECT
public:
    Thumbnail(QPixmap pixmap, QWidget *parent);
    void mousePressEvent(QMouseEvent *ev);
    void select(bool selected);
    // Variables
    QPixmap photo;
signals:
    void clicked(QPixmap);
};

// ThumbnailGroup contains one or more thumbnails, it is used to select a thumbnail
class ThumbnailGroup : public QObject
{
    Q_OBJECT
public:
    ThumbnailGroup(QObject *parent);
    void append(Thumbnail *thumbnail);
    // Variables
    QList<Thumbnail *> thumbnails; 
public slots:
    void selectThumbnail(QPixmap);
};

// The paper on which collage is created and displayed.
class GridPaper : public QLabel
{
    Q_OBJECT
public:
    GridPaper(QWidget *parent);
    void setupGrid();
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void createFinalGrid();
    // Variables
    //float scale;
    int paperW, paperH, W, H, cols, rows, spacingX, spacingY;
    QList<QRect> boxes;
    QMap<int, QPixmap> pixmap_dict;
    QPixmap photo, photo_grid;
public slots:
    void setPhoto(QPixmap pixmap);
};

// The dialog to create the grid
class GridDialog : public QDialog, public Ui_GridDialog
{
    Q_OBJECT
public:
    GridDialog(QPixmap pixmap, QWidget *parent);
    void accept();
    // Variables
    GridPaper *gridPaper;
    ThumbnailGroup *thumbnailGr;
public slots:
    void addPhoto();
    void showHelp();
};

// Static functions
QPixmap loadImage(QString filename);  // Returns an autorotated image according to exif data

QT_END_NAMESPACE
#endif