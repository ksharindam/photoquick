/********************************************************************************
** Form generated from reading UI file 'photogrid-dialog.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PHOTOGRID_2D_DIALOG_H
#define UI_PHOTOGRID_2D_DIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QScrollArea>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GridDialog
{
public:
    QGridLayout *gridLayout;
    QFrame *frame;
    QVBoxLayout *verticalLayout;
    QPushButton *addPhotoBtn;
    QSpacerItem *verticalSpacer;
    QCheckBox *checkAddBorder;
    QDialogButtonBox *buttonBox;
    QPushButton *helpBtn;
    QPushButton *configureBtn;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;

    void setupUi(QDialog *GridDialog)
    {
        if (GridDialog->objectName().isEmpty())
            GridDialog->setObjectName(QString::fromUtf8("GridDialog"));
        GridDialog->resize(657, 469);
        gridLayout = new QGridLayout(GridDialog);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setHorizontalSpacing(0);
        frame = new QFrame(GridDialog);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout = new QVBoxLayout(frame);
        verticalLayout->setContentsMargins(2, 2, 2, 2);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        addPhotoBtn = new QPushButton(frame);
        addPhotoBtn->setObjectName(QString::fromUtf8("addPhotoBtn"));

        verticalLayout->addWidget(addPhotoBtn);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        gridLayout->addWidget(frame, 0, 0, 1, 1);

        checkAddBorder = new QCheckBox(GridDialog);
        checkAddBorder->setObjectName(QString::fromUtf8("checkAddBorder"));
        checkAddBorder->setChecked(true);

        gridLayout->addWidget(checkAddBorder, 2, 2, 1, 1);

        buttonBox = new QDialogButtonBox(GridDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 2, 3, 1, 1);

        helpBtn = new QPushButton(GridDialog);
        helpBtn->setObjectName(QString::fromUtf8("helpBtn"));

        gridLayout->addWidget(helpBtn, 2, 0, 1, 1);

        configureBtn = new QPushButton(GridDialog);
        configureBtn->setObjectName(QString::fromUtf8("configureBtn"));

        gridLayout->addWidget(configureBtn, 2, 1, 1, 1);

        scrollArea = new QScrollArea(GridDialog);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        scrollArea->setWidgetResizable(true);
        scrollArea->setAlignment(Qt::AlignCenter);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 561, 431));
        scrollArea->setWidget(scrollAreaWidgetContents);

        gridLayout->addWidget(scrollArea, 0, 1, 1, 3);


        retranslateUi(GridDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), GridDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), GridDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(GridDialog);
    } // setupUi

    void retranslateUi(QDialog *GridDialog)
    {
        GridDialog->setWindowTitle(QApplication::translate("GridDialog", "Create Photo Grid", 0, QApplication::UnicodeUTF8));
        addPhotoBtn->setText(QApplication::translate("GridDialog", "Add Photo", 0, QApplication::UnicodeUTF8));
        checkAddBorder->setText(QApplication::translate("GridDialog", "Add Border", 0, QApplication::UnicodeUTF8));
        helpBtn->setText(QApplication::translate("GridDialog", "Help", 0, QApplication::UnicodeUTF8));
        configureBtn->setText(QApplication::translate("GridDialog", "Configure", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class GridDialog: public Ui_GridDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PHOTOGRID_2D_DIALOG_H
