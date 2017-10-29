/********************************************************************************
** Form generated from reading UI file 'resize_dialog.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RESIZE_DIALOG_H
#define UI_RESIZE_DIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QSpinBox>

QT_BEGIN_NAMESPACE

class Ui_ResizeDialog
{
public:
    QGridLayout *gridLayout_2;
    QLabel *label;
    QLabel *label_2;
    QDialogButtonBox *buttonBox;
    QFrame *frame;
    QGridLayout *gridLayout;
    QLabel *label_5;
    QLabel *label_3;
    QDoubleSpinBox *spinWidth;
    QLabel *label_4;
    QSpinBox *spinDPI;
    QDoubleSpinBox *spinHeight;
    QCheckBox *checkBox;
    QLineEdit *widthEdit;
    QLineEdit *heightEdit;

    void setupUi(QDialog *ResizeDialog)
    {
        if (ResizeDialog->objectName().isEmpty())
            ResizeDialog->setObjectName(QString::fromUtf8("ResizeDialog"));
        ResizeDialog->resize(353, 254);
        gridLayout_2 = new QGridLayout(ResizeDialog);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label = new QLabel(ResizeDialog);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout_2->addWidget(label, 0, 0, 1, 1);

        label_2 = new QLabel(ResizeDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout_2->addWidget(label_2, 1, 0, 1, 1);

        buttonBox = new QDialogButtonBox(ResizeDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout_2->addWidget(buttonBox, 4, 0, 1, 2);

        frame = new QFrame(ResizeDialog);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Plain);
        gridLayout = new QGridLayout(frame);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_5 = new QLabel(frame);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 2, 0, 1, 1);

        label_3 = new QLabel(frame);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 0, 0, 1, 1);

        spinWidth = new QDoubleSpinBox(frame);
        spinWidth->setObjectName(QString::fromUtf8("spinWidth"));
        spinWidth->setAlignment(Qt::AlignCenter);
        spinWidth->setDecimals(1);
        spinWidth->setSingleStep(0.1);

        gridLayout->addWidget(spinWidth, 1, 1, 1, 1);

        label_4 = new QLabel(frame);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 1, 0, 1, 1);

        spinDPI = new QSpinBox(frame);
        spinDPI->setObjectName(QString::fromUtf8("spinDPI"));
        spinDPI->setAlignment(Qt::AlignCenter);
        spinDPI->setMinimum(75);
        spinDPI->setMaximum(1000);
        spinDPI->setValue(300);

        gridLayout->addWidget(spinDPI, 0, 1, 1, 1);

        spinHeight = new QDoubleSpinBox(frame);
        spinHeight->setObjectName(QString::fromUtf8("spinHeight"));
        spinHeight->setAlignment(Qt::AlignCenter);
        spinHeight->setDecimals(1);
        spinHeight->setSingleStep(0.1);

        gridLayout->addWidget(spinHeight, 2, 1, 1, 1);


        gridLayout_2->addWidget(frame, 3, 0, 1, 2);

        checkBox = new QCheckBox(ResizeDialog);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));

        gridLayout_2->addWidget(checkBox, 2, 0, 1, 2);

        widthEdit = new QLineEdit(ResizeDialog);
        widthEdit->setObjectName(QString::fromUtf8("widthEdit"));
        widthEdit->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(widthEdit, 0, 1, 1, 1);

        heightEdit = new QLineEdit(ResizeDialog);
        heightEdit->setObjectName(QString::fromUtf8("heightEdit"));
        heightEdit->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(heightEdit, 1, 1, 1, 1);


        retranslateUi(ResizeDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), ResizeDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ResizeDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(ResizeDialog);
    } // setupUi

    void retranslateUi(QDialog *ResizeDialog)
    {
        ResizeDialog->setWindowTitle(QApplication::translate("ResizeDialog", "Resize Image", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ResizeDialog", "Width :", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("ResizeDialog", "Height :", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("ResizeDialog", "Height :", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("ResizeDialog", "DPI :", 0, QApplication::UnicodeUTF8));
        spinWidth->setSuffix(QApplication::translate("ResizeDialog", " cm", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("ResizeDialog", "Width :", 0, QApplication::UnicodeUTF8));
        spinHeight->setSuffix(QApplication::translate("ResizeDialog", " cm", 0, QApplication::UnicodeUTF8));
        checkBox->setText(QApplication::translate("ResizeDialog", "Show Advanced (DPI and  cm)", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ResizeDialog: public Ui_ResizeDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RESIZE_DIALOG_H
