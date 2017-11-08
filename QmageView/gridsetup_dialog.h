/********************************************************************************
** Form generated from reading UI file 'gridsetup-dialog.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GRIDSETUP_2D_DIALOG_H
#define UI_GRIDSETUP_2D_DIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>

QT_BEGIN_NAMESPACE

class Ui_GridSetupDialog
{
public:
    QGridLayout *gridLayout;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QSpinBox *spinPaperWidth;
    QDoubleSpinBox *spinPhotoWidth;
    QLabel *label_5;
    QLabel *label_6;
    QSpinBox *spinPaperHeight;
    QDoubleSpinBox *spinPhotoHeight;
    QSpinBox *spinDPI;
    QLabel *label_4;
    QComboBox *paperSizeUnit;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *GridSetupDialog)
    {
        if (GridSetupDialog->objectName().isEmpty())
            GridSetupDialog->setObjectName(QString::fromUtf8("GridSetupDialog"));
        GridSetupDialog->setWindowModality(Qt::WindowModal);
        GridSetupDialog->resize(441, 240);
        gridLayout = new QGridLayout(GridSetupDialog);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(GridSetupDialog);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        label_2 = new QLabel(GridSetupDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        label_3 = new QLabel(GridSetupDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        spinPaperWidth = new QSpinBox(GridSetupDialog);
        spinPaperWidth->setObjectName(QString::fromUtf8("spinPaperWidth"));
        spinPaperWidth->setAlignment(Qt::AlignCenter);
        spinPaperWidth->setMinimum(1);
        spinPaperWidth->setMaximum(500);
        spinPaperWidth->setValue(6);

        gridLayout->addWidget(spinPaperWidth, 0, 1, 1, 1);

        spinPhotoWidth = new QDoubleSpinBox(GridSetupDialog);
        spinPhotoWidth->setObjectName(QString::fromUtf8("spinPhotoWidth"));
        spinPhotoWidth->setAlignment(Qt::AlignCenter);
        spinPhotoWidth->setDecimals(1);
        spinPhotoWidth->setMinimum(1);
        spinPhotoWidth->setSingleStep(0.5);
        spinPhotoWidth->setValue(3.5);

        gridLayout->addWidget(spinPhotoWidth, 2, 1, 1, 1);

        label_5 = new QLabel(GridSetupDialog);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setMaximumSize(QSize(10, 16777215));

        gridLayout->addWidget(label_5, 0, 2, 1, 1);

        label_6 = new QLabel(GridSetupDialog);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 2, 2, 1, 1);

        spinPaperHeight = new QSpinBox(GridSetupDialog);
        spinPaperHeight->setObjectName(QString::fromUtf8("spinPaperHeight"));
        spinPaperHeight->setAlignment(Qt::AlignCenter);
        spinPaperHeight->setMinimum(1);
        spinPaperHeight->setMaximum(500);
        spinPaperHeight->setValue(4);

        gridLayout->addWidget(spinPaperHeight, 0, 3, 1, 1);

        spinPhotoHeight = new QDoubleSpinBox(GridSetupDialog);
        spinPhotoHeight->setObjectName(QString::fromUtf8("spinPhotoHeight"));
        spinPhotoHeight->setAlignment(Qt::AlignCenter);
        spinPhotoHeight->setDecimals(1);
        spinPhotoHeight->setMinimum(1);
        spinPhotoHeight->setSingleStep(0.5);
        spinPhotoHeight->setValue(4.5);

        gridLayout->addWidget(spinPhotoHeight, 2, 3, 1, 1);

        spinDPI = new QSpinBox(GridSetupDialog);
        spinDPI->setObjectName(QString::fromUtf8("spinDPI"));
        spinDPI->setAlignment(Qt::AlignCenter);
        spinDPI->setMinimum(100);
        spinDPI->setMaximum(1200);
        spinDPI->setValue(300);

        gridLayout->addWidget(spinDPI, 1, 1, 1, 3);

        label_4 = new QLabel(GridSetupDialog);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 2, 4, 1, 1);

        paperSizeUnit = new QComboBox(GridSetupDialog);
        paperSizeUnit->setObjectName(QString::fromUtf8("paperSizeUnit"));

        gridLayout->addWidget(paperSizeUnit, 0, 4, 1, 1);

        buttonBox = new QDialogButtonBox(GridSetupDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 3, 0, 1, 5);


        retranslateUi(GridSetupDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), GridSetupDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), GridSetupDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(GridSetupDialog);
    } // setupUi

    void retranslateUi(QDialog *GridSetupDialog)
    {
        GridSetupDialog->setWindowTitle(QApplication::translate("GridSetupDialog", "Setup Grid", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("GridSetupDialog", "Paper Size :", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("GridSetupDialog", "DPI :", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("GridSetupDialog", "Photo Size :", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("GridSetupDialog", "x", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("GridSetupDialog", "x", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("GridSetupDialog", "cm", 0, QApplication::UnicodeUTF8));
        paperSizeUnit->clear();
        paperSizeUnit->insertItems(0, QStringList()
         << QApplication::translate("GridSetupDialog", "inch", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("GridSetupDialog", "cm", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("GridSetupDialog", "mm", 0, QApplication::UnicodeUTF8)
        );
    } // retranslateUi

};

namespace Ui {
    class GridSetupDialog: public Ui_GridSetupDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GRIDSETUP_2D_DIALOG_H
