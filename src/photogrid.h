#pragma once
/*
This file is a part of qmageview program, which is GPLv3 licensed
*/

#include "ui_photogrid_dialog.h"
#include "ui_gridsetup_dialog.h"
#include <QLabel>
#include <QMouseEvent>

// Thumbnail class holds a photo
class Thumbnail : public QLabel
{
    Q_OBJECT
public:
    Thumbnail(QImage img, QWidget *parent);
    void mousePressEvent(QMouseEvent *ev);
    void select(bool selected);
    // Variables
    QImage photo;
signals:
    void clicked(QImage);
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
    void selectThumbnail(QImage);
};

// The canvas on which collage is created and displayed.
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
    int DPI, paperW, paperH, W, H, cols, rows;
    float scale, spacingX, spacingY;
    bool add_border;
    QList<QRect> boxes;
    QMap<int, QImage> image_dict;
    QImage photo, photo_grid;
public slots:
    void setPhoto(QImage img);
    void toggleBorder(bool ok);
};

// The dialog to create the grid
class GridDialog : public QDialog, public Ui_GridDialog
{
    Q_OBJECT
public:
    GridDialog(QImage img, QWidget *parent);
    void accept();
    // Variables
    GridPaper *gridPaper;
    ThumbnailGroup *thumbnailGr;
public slots:
    void configure();
    void addPhoto();
    void showHelp();
};

class GridSetupDialog : public QDialog, public Ui_GridSetupDialog
{
public:
    GridSetupDialog(QWidget *parent);
    void accept();
    int W, H, DPI, rows, cols, paperW, paperH;
};

// Static functions
QImage loadImage(QString filename);  // Returns an autorotated image according to exif data

