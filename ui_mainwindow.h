/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include "QmyLabel.h"
#include "fileOptions.h"
#include "resizeImage.h"
#include "cropImage.h"
#include "addBorder.h"

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QScrollArea>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <QtGui/QStatusBar>
#include <QtGui/QKeySequence>
#include <QIcon>
#include <QSettings>

QT_BEGIN_NAMESPACE

class Ui_MainWindow : public QObject
{
    Q_OBJECT
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout, *gridLayout_2;
    QVBoxLayout *verticalLayout, *verticalLayout_2;
    QPushButton *openButton, *saveButton, *quitButton, *nextButton, *prevButton, *zoomButton, *zoomoutButton, *originalsizeButton, *rotateleftButton;
    QPushButton *resizeButton, *cropButton, *borderButton, *rotaterightButton, *gridButton, *slideshowButton;
    QSpacerItem *verticalSpacer, *verticalSpacer_3;
    QScrollArea *scrollArea;
    QWidget *scrollbox;
    QmyLabel *label;
    QPixmap pm, pm_scaled;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
//        MainWindow->resize(640, 480);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(centralwidget->sizePolicy().hasHeightForWidth());
        centralwidget->setSizePolicy(sizePolicy);
        gridLayout_2 = new QGridLayout(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        MainWindow->setStatusBar(statusbar);
        gridLayout_2->setSpacing(0);
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        
        // First Button box with edit buttons
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        verticalLayout->setSpacing(14);

        openButton = new QPushButton(QIcon(":/open.png"), "",centralwidget);
        openButton->setObjectName(QString::fromUtf8("openButton"));
        openButton->setToolTip("Open File \n [Ctrl+O]");
        openButton->setIconSize(QSize(32,32));
        verticalLayout->addWidget(openButton);

        saveButton = new QPushButton(QIcon(":/save.png"), "", centralwidget);
        saveButton->setObjectName(QString::fromUtf8("saveButton"));
        saveButton->setIconSize(QSize(32,32));
        saveButton->setToolTip("Save File \n [Ctrl+S]");
        verticalLayout->addWidget(saveButton);


        resizeButton = new QPushButton(QIcon(":/resize.png"), "", centralwidget);
        resizeButton->setObjectName(QString::fromUtf8("resizeButton"));
        resizeButton->setIconSize(QSize(32,32));
        resizeButton->setToolTip("Resize Image");
        verticalLayout->addWidget(resizeButton);

        cropButton = new QPushButton(QIcon(":/crop.png"), "", centralwidget);
        cropButton->setObjectName(QString::fromUtf8("cropButton"));
        cropButton->setIconSize(QSize(32,32));
        cropButton->setToolTip("Crop Image");
        verticalLayout->addWidget(cropButton);

        borderButton = new QPushButton(QIcon(":/addborder.png"), "", centralwidget);
        borderButton->setObjectName(QString::fromUtf8("borderButton"));
        borderButton->setIconSize(QSize(32,32));
        borderButton->setToolTip("Add Border");
        verticalLayout->addWidget(borderButton);

        gridButton = new QPushButton(QIcon(":/creategrid.png"), "", centralwidget);
        gridButton->setIconSize(QSize(32,32));
        gridButton->setObjectName(QString::fromUtf8("gridButton"));
        gridButton->setToolTip("Create 4x6 inch grid \n with 8 images");
        verticalLayout->addWidget(gridButton);

        verticalSpacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
        verticalLayout->addItem(verticalSpacer);

        quitButton = new QPushButton(QIcon(":/quit.png"), "", centralwidget);
        quitButton->setObjectName(QString::fromUtf8("quitButton"));
        quitButton->setIconSize(QSize(32,32));
        quitButton->setToolTip("Close Application");
        verticalLayout->addWidget(quitButton);


//      Second Button Box that holds view buttons
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setSizeConstraint(QLayout::SetMinAndMaxSize);
        verticalLayout_2->setSpacing(6);

        prevButton = new QPushButton(QIcon(":/prev.png"), "", centralwidget);
        prevButton->setObjectName(QString::fromUtf8("prevButton"));
        prevButton->setIconSize(QSize(32,32));
        prevButton->setToolTip("Open Previous File \n [Left-Arrow]");
        verticalLayout_2->addWidget(prevButton);

        nextButton = new QPushButton(QIcon(":/next.png"), "", centralwidget);
        nextButton->setObjectName(QString::fromUtf8("nextButton"));
        nextButton->setIconSize(QSize(32,32));
        nextButton->setToolTip("Open Next File \n [Right-Arrow]");
        verticalLayout_2->addWidget(nextButton);

        zoomButton = new QPushButton(QIcon(":/zoomin.png"), "", centralwidget);
        zoomButton->setObjectName(QString::fromUtf8("zoomButton"));
        zoomButton->setIconSize(QSize(32,32));
        zoomButton->setToolTip("Zoom in Image \n [+]");
        verticalLayout_2->addWidget(zoomButton);

        zoomoutButton = new QPushButton(QIcon(":/zoomout.png"), "", centralwidget);
        zoomoutButton->setObjectName(QString::fromUtf8("zoomoutButton"));
        zoomoutButton->setIconSize(QSize(32,32));
        zoomoutButton->setToolTip("Zoom Out \n [-]");
        verticalLayout_2->addWidget(zoomoutButton);

        originalsizeButton = new QPushButton(QIcon(":/originalsize.png"), "", centralwidget);
        originalsizeButton->setObjectName(QString::fromUtf8("originalsizeButton"));
        originalsizeButton->setIconSize(QSize(32,32));
        originalsizeButton->setToolTip("Scale to 1x");
        verticalLayout_2->addWidget(originalsizeButton);

        rotateleftButton = new QPushButton(QIcon(":/rotateleft.png"), "", centralwidget);
        rotateleftButton->setIconSize(QSize(32,32));
        rotateleftButton->setToolTip("Rotate Left");
        verticalLayout_2->addWidget(rotateleftButton);

        rotaterightButton = new QPushButton(QIcon(":/rotateright.png"), "", centralwidget);
        rotaterightButton->setIconSize(QSize(32,32));
        rotaterightButton->setToolTip("Rotate Right");
        verticalLayout_2->addWidget(rotaterightButton);

        verticalSpacer_3 = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
        verticalLayout_2->addItem(verticalSpacer_3);

        slideshowButton = new QPushButton(QIcon(":/play.png"), "", centralwidget);
        slideshowButton->setIconSize(QSize(32,32));
        slideshowButton->setToolTip("Slide Show");
        verticalLayout_2->addWidget(slideshowButton);


        scrollArea = new QScrollArea(centralwidget);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        sizePolicy.setHeightForWidth(scrollArea->sizePolicy().hasHeightForWidth());
        scrollArea->setSizePolicy(sizePolicy);
        scrollArea->setMinimumSize(QSize(600, 400));
        scrollArea->setWidgetResizable(true);
        scrollArea->setAlignment(Qt::AlignCenter);
        scrollbox = new QWidget();
        scrollbox->setObjectName(QString::fromUtf8("scrollbox"));
        sizePolicy.setHeightForWidth(scrollbox->sizePolicy().hasHeightForWidth());
        scrollbox->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(scrollbox);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        label = new QmyLabel(scrollbox);
        label->setObjectName(QString::fromUtf8("label"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy1);
        label->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label, 0, 0, 1, 1);

        scrollArea->setWidget(scrollbox);

        gridLayout_2->addLayout(verticalLayout, 0, 0, 1, 1);
        gridLayout_2->addWidget(scrollArea, 0, 1, 1, 1);
        gridLayout_2->addLayout(verticalLayout_2, 0, 2, 1, 1);


        MainWindow->setCentralWidget(centralwidget);

        // This is placed after all widgets havebeen created.Otherwise don't work
        quitButton->setShortcut(QString("ESC"));
        openButton->setShortcut(QString("Ctrl+O"));
        saveButton->setShortcut(QString("Ctrl+S"));
        nextButton->setShortcut(QString("Right"));
        prevButton->setShortcut(QString("Left"));
        slideshowButton->setShortcut(QString("Space"));
        zoomButton->setShortcut(QString("+"));
        zoomoutButton->setShortcut(QString("-"));

        QObject::connect(quitButton, SIGNAL(clicked()), MainWindow, SLOT(close()));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi


};

class MyWindow : public QMainWindow
{
    void closeEvent(QCloseEvent*);
};

namespace Ui {
    class MainWindow: public Ui_MainWindow
{
    Q_OBJECT
public:
    fileOptions *fopt;
    resizer *resimg;
    cropper *cropimg;
    bordering *borderadd;
    qreal *scale;
    void setupConfig(QMainWindow *MainWindow);
    ~MainWindow();
    QMainWindow *win;
    QSettings *settings;
};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
