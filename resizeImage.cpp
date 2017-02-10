#include "resizeImage.h"
#include <QIntValidator>


// resizeimage slot constructor
resizer::resizer (QMainWindow *mainWindow, QPixmap *pixmap, QPixmap *pixmap_scaled, float *Scale, QLabel *imagelabel, QStatusBar *status)
{
    pm = pixmap;
    pm_scaled = pixmap_scaled;
    label = imagelabel;
    scale = Scale;
    MainWindow = mainWindow;
    statusbar = status;
}

// 1st Slot to resize image, that creates dialog.
void resizer::resizeimage ()
{
    QDialog *dialog = new QDialog(MainWindow);
    ResizeDialog resDialog;
    resDialog.setupUi(dialog);
    if (dialog->exec() == QDialog::Accepted)
    {
    QString widthtext = resDialog.widthEdit->text();
    QString heighttext = resDialog.heightEdit->text();
    int imgwidth = widthtext.toInt();
    int imgheight = heighttext.toInt();
    
    if (!widthtext.isEmpty() && !heighttext.isEmpty())
      *pm = pm->scaled(imgwidth,imgheight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
      
    else if (!widthtext.isEmpty())
      *pm = pm->scaledToWidth(imgwidth, Qt::SmoothTransformation);
      
    else if (!heighttext.isEmpty())
      *pm = pm->scaledToHeight(imgheight, Qt::SmoothTransformation);

    *pm_scaled = pm->scaledToHeight(*scale*pm->height(), Qt::SmoothTransformation);
    label->setPixmap(*pm_scaled);
    statusbar->showMessage(QString("Resolution : %1x%2 , Scale : %3x").arg(pm->width()).arg(pm->height()).arg(*scale));
    }
}



void ResizeDialog::setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
        Dialog->setObjectName("ResDialog");
        Dialog->resize(296, 133);
        Dialog->setModal(true);
        Dialog->setWindowTitle("Resize Image");
        gridLayout = new QGridLayout(Dialog);

        buttonBox = new QDialogButtonBox(Dialog);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        gridLayout->addWidget(buttonBox, 1, 0, 1, 1);

        gridLayout_2 = new QGridLayout();

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        gridLayout_2->addItem(verticalSpacer_3, 5, 1, 1, 1);

        label_2 = new QLabel(Dialog);
        label_2->setText("Width");
        gridLayout_2->addWidget(label_2, 3, 0, 1, 1);

        label = new QLabel(Dialog);
        label->setText("Height");
        gridLayout_2->addWidget(label, 4, 0, 1, 1);

        QIntValidator *validator = new QIntValidator(Dialog);
        widthEdit = new QLineEdit(Dialog);
        widthEdit->setValidator(validator);
        gridLayout_2->addWidget(widthEdit, 3, 1, 1, 1);

        heightEdit = new QLineEdit(Dialog);
        heightEdit->setValidator(validator);
        gridLayout_2->addWidget(heightEdit, 4, 1, 1, 1);


        gridLayout->addLayout(gridLayout_2, 0, 0, 1, 1);


        QObject::connect(buttonBox, SIGNAL(accepted()), Dialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), Dialog, SLOT(reject()));

        widthEdit->setFocusPolicy(Qt::StrongFocus);
        widthEdit->setFocus();
} // setupUi

