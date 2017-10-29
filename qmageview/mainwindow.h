/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QScrollArea>
#include <QtGui/QSpacerItem>
#include <QtGui/QStatusBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QFrame *frame;
    QVBoxLayout *verticalLayout;
    QPushButton *openBtn;
    QPushButton *saveBtn;
    QPushButton *resizeBtn;
    QPushButton *cropBtn;
    QPushButton *addBorderBtn;
    QPushButton *photoGridBtn;
    QSpacerItem *verticalSpacer;
    QPushButton *quitBtn;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QFrame *frame_2;
    QVBoxLayout *verticalLayout_2;
    QPushButton *prevBtn;
    QPushButton *nextBtn;
    QPushButton *zoomInBtn;
    QPushButton *zoomOutBtn;
    QPushButton *origSizeBtn;
    QPushButton *rotateLeftBtn;
    QPushButton *rotateRightBtn;
    QSpacerItem *verticalSpacer_2;
    QPushButton *slideShowBtn;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(613, 409);
        MainWindow->setMinimumSize(QSize(613, 409));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        frame = new QFrame(centralwidget);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout = new QVBoxLayout(frame);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        openBtn = new QPushButton(frame);
        openBtn->setObjectName(QString::fromUtf8("openBtn"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/open.png"), QSize(), QIcon::Normal, QIcon::Off);
        openBtn->setIcon(icon);
        openBtn->setIconSize(QSize(32, 32));

        verticalLayout->addWidget(openBtn);

        saveBtn = new QPushButton(frame);
        saveBtn->setObjectName(QString::fromUtf8("saveBtn"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/save.png"), QSize(), QIcon::Normal, QIcon::Off);
        saveBtn->setIcon(icon1);
        saveBtn->setIconSize(QSize(32, 32));

        verticalLayout->addWidget(saveBtn);

        resizeBtn = new QPushButton(frame);
        resizeBtn->setObjectName(QString::fromUtf8("resizeBtn"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/images/resize.png"), QSize(), QIcon::Normal, QIcon::Off);
        resizeBtn->setIcon(icon2);
        resizeBtn->setIconSize(QSize(32, 32));

        verticalLayout->addWidget(resizeBtn);

        cropBtn = new QPushButton(frame);
        cropBtn->setObjectName(QString::fromUtf8("cropBtn"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/images/crop.png"), QSize(), QIcon::Normal, QIcon::Off);
        cropBtn->setIcon(icon3);
        cropBtn->setIconSize(QSize(32, 32));

        verticalLayout->addWidget(cropBtn);

        addBorderBtn = new QPushButton(frame);
        addBorderBtn->setObjectName(QString::fromUtf8("addBorderBtn"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/images/addborder.png"), QSize(), QIcon::Normal, QIcon::Off);
        addBorderBtn->setIcon(icon4);
        addBorderBtn->setIconSize(QSize(32, 32));

        verticalLayout->addWidget(addBorderBtn);

        photoGridBtn = new QPushButton(frame);
        photoGridBtn->setObjectName(QString::fromUtf8("photoGridBtn"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/images/creategrid.png"), QSize(), QIcon::Normal, QIcon::Off);
        photoGridBtn->setIcon(icon5);
        photoGridBtn->setIconSize(QSize(32, 32));

        verticalLayout->addWidget(photoGridBtn);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        quitBtn = new QPushButton(frame);
        quitBtn->setObjectName(QString::fromUtf8("quitBtn"));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/images/quit.png"), QSize(), QIcon::Normal, QIcon::Off);
        quitBtn->setIcon(icon6);
        quitBtn->setIconSize(QSize(32, 32));

        verticalLayout->addWidget(quitBtn);


        horizontalLayout->addWidget(frame);

        scrollArea = new QScrollArea(centralwidget);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 509, 384));
        scrollArea->setWidget(scrollAreaWidgetContents);

        horizontalLayout->addWidget(scrollArea);

        frame_2 = new QFrame(centralwidget);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);
        verticalLayout_2 = new QVBoxLayout(frame_2);
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        prevBtn = new QPushButton(frame_2);
        prevBtn->setObjectName(QString::fromUtf8("prevBtn"));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/images/prev.png"), QSize(), QIcon::Normal, QIcon::Off);
        prevBtn->setIcon(icon7);
        prevBtn->setIconSize(QSize(32, 32));

        verticalLayout_2->addWidget(prevBtn);

        nextBtn = new QPushButton(frame_2);
        nextBtn->setObjectName(QString::fromUtf8("nextBtn"));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/images/next.png"), QSize(), QIcon::Normal, QIcon::Off);
        nextBtn->setIcon(icon8);
        nextBtn->setIconSize(QSize(32, 32));

        verticalLayout_2->addWidget(nextBtn);

        zoomInBtn = new QPushButton(frame_2);
        zoomInBtn->setObjectName(QString::fromUtf8("zoomInBtn"));
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/images/zoomin.png"), QSize(), QIcon::Normal, QIcon::Off);
        zoomInBtn->setIcon(icon9);
        zoomInBtn->setIconSize(QSize(32, 32));

        verticalLayout_2->addWidget(zoomInBtn);

        zoomOutBtn = new QPushButton(frame_2);
        zoomOutBtn->setObjectName(QString::fromUtf8("zoomOutBtn"));
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/images/zoomout.png"), QSize(), QIcon::Normal, QIcon::Off);
        zoomOutBtn->setIcon(icon10);
        zoomOutBtn->setIconSize(QSize(32, 32));

        verticalLayout_2->addWidget(zoomOutBtn);

        origSizeBtn = new QPushButton(frame_2);
        origSizeBtn->setObjectName(QString::fromUtf8("origSizeBtn"));
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/images/originalsize.png"), QSize(), QIcon::Normal, QIcon::Off);
        origSizeBtn->setIcon(icon11);
        origSizeBtn->setIconSize(QSize(32, 32));

        verticalLayout_2->addWidget(origSizeBtn);

        rotateLeftBtn = new QPushButton(frame_2);
        rotateLeftBtn->setObjectName(QString::fromUtf8("rotateLeftBtn"));
        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/images/rotateleft.png"), QSize(), QIcon::Normal, QIcon::Off);
        rotateLeftBtn->setIcon(icon12);
        rotateLeftBtn->setIconSize(QSize(32, 32));

        verticalLayout_2->addWidget(rotateLeftBtn);

        rotateRightBtn = new QPushButton(frame_2);
        rotateRightBtn->setObjectName(QString::fromUtf8("rotateRightBtn"));
        QIcon icon13;
        icon13.addFile(QString::fromUtf8(":/images/rotateright.png"), QSize(), QIcon::Normal, QIcon::Off);
        rotateRightBtn->setIcon(icon13);
        rotateRightBtn->setIconSize(QSize(32, 32));

        verticalLayout_2->addWidget(rotateRightBtn);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_2);

        slideShowBtn = new QPushButton(frame_2);
        slideShowBtn->setObjectName(QString::fromUtf8("slideShowBtn"));
        QIcon icon14;
        icon14.addFile(QString::fromUtf8(":/images/play.png"), QSize(), QIcon::Normal, QIcon::Off);
        slideShowBtn->setIcon(icon14);
        slideShowBtn->setIconSize(QSize(32, 32));
        slideShowBtn->setCheckable(true);

        verticalLayout_2->addWidget(slideShowBtn);


        horizontalLayout->addWidget(frame_2);

        horizontalLayout->setStretch(1, 1);
        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Image Viewer", 0, QApplication::UnicodeUTF8));
        openBtn->setText(QString());
        saveBtn->setText(QString());
        resizeBtn->setText(QString());
        cropBtn->setText(QString());
        addBorderBtn->setText(QString());
        photoGridBtn->setText(QString());
        quitBtn->setText(QString());
        prevBtn->setText(QString());
        nextBtn->setText(QString());
        zoomInBtn->setText(QString());
        zoomOutBtn->setText(QString());
        origSizeBtn->setText(QString());
        rotateLeftBtn->setText(QString());
        rotateRightBtn->setText(QString());
        slideShowBtn->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
