/********************************************************************************
** Form generated from reading UI file 'pic-gridT13036.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef PIC_2D_GRIDT13036_H
#define PIC_2D_GRIDT13036_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QPixmap>
#include <iostream>
#include "QmyLabel.h"

#include <QObject>
#include <QMainWindow>
#include <QFrame>
#include <QFileDialog>
#include <QPainter>

QT_BEGIN_NAMESPACE

 class Thumbnail : public QLabel
 // This is the image label which are added to ImageHolder frame
 {
     Q_OBJECT
 public:
     QPixmap originalPixmap;
     Thumbnail(QWidget *parent=0) ;
     void addPixmap(QPixmap pm);
signals:
     void clicked( QPixmap );
 protected:
//     void dragEnterEvent(QDragEnterEvent *event);
//     void dragMoveEvent(QDragMoveEvent *event);
     void mousePressEvent(QMouseEvent *event);
 };

class ImageHolder : public QFrame
// It holds multiple images and 'add image' button
{
    Q_OBJECT
public:
    QVBoxLayout *verticalLayout;
    ImageHolder(QWidget *parent=0);
    void addImage(QPixmap);
    void addwidget(QWidget *);
    void addspacer(QSpacerItem *);
signals:
    void clicked(QPixmap);
public slots:
    void loadImage();
    void thumbnailClicked( QPixmap ); // emits above signal to send pixmap to ImageGrid
};

 class ImageGrid : public QLabel
 // This is the image grid label
 {
    Q_OBJECT
 public:
     QPixmap source_pixmap, hidden_pixmap;
     qreal pg_width, pg_height, img_width, img_height, scale;
     ImageGrid(QWidget *parent=0) ;
     void setBackground(QPixmap pm);
     void snapPixmap(int point_x, int point_y);
 signals:
    void gridUpdated(QPixmap);
 public slots:
     void setSourcePixmap(QPixmap);
 protected:
     void dragEnterEvent(QDragEnterEvent *event);
     void dragMoveEvent(QDragMoveEvent *event);
     void dropEvent(QDropEvent *event);
 };

class GridMaker : public QObject
// This has a public slot that opens grid maker dialog
{
    Q_OBJECT
private:
    QMainWindow *mainwindow;
    QPixmap *pixmap, *pixmap_scaled;
    qreal *scale;
    QmyLabel *label;
public:
    GridMaker(QMainWindow *MainWindow, QPixmap *pixmap, QPixmap *pixmap_scaled, float *Scale, QmyLabel *imagelabel);
public slots:
    void createGrid();
};


class Grid_Dialog : public QObject
{
public:
    QGridLayout *gridLayout;
    ImageHolder *frame;
    QPushButton *pushButton;
    QSpacerItem *verticalSpacer;
    QVBoxLayout *verticalLayout_2;
    ImageGrid *label_2;
    QDialogButtonBox *buttonBox;
    QPixmap pixmap;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QString::fromUtf8("Dialog"));
        Dialog->resize(634, 502);
        gridLayout = new QGridLayout(Dialog);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        frame = new ImageHolder(Dialog);
        frame->setObjectName(QString::fromUtf8("frame_2"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        gridLayout->addWidget(frame, 0, 0, 1, 1);

        pushButton = new QPushButton(Dialog);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        frame->addwidget(pushButton);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        frame->addspacer(verticalSpacer);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        label_2 = new ImageGrid(Dialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setMouseTracking(true);
        label_2->setAcceptDrops(true);


        verticalLayout_2->addWidget(label_2, 0, Qt::AlignHCenter|Qt::AlignVCenter);


        gridLayout->addLayout(verticalLayout_2, 0, 1, 1, 1);

        buttonBox = new QDialogButtonBox(Dialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 1, 1, 1, 1);


        retranslateUi(Dialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), Dialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), Dialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("Dialog", "Dialog", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("Dialog", "Add Image", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

/* class QDragEnterEvent;
 class QDropEvent;*/

namespace Grid {

    class Dialog: public Grid_Dialog
{
    Q_OBJECT
public:
    QPixmap image_grid;
    void configUi(QPixmap);
public slots:
    void setGrid(QPixmap);
};

} // namespace Ui

QT_END_NAMESPACE

#endif // PIC_2D_GRIDT13036_H
