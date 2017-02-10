#include "addBorder.h"


bordering::bordering (QMainWindow *mainWindow, QPixmap *pixmap, QPixmap *pixmap_scaled, float *Scale, QmyLabel *imagelabel)
{
    pm = pixmap;
    pm_scaled = pixmap_scaled;
    label = imagelabel;
    scale = Scale;
    MainWindow = mainWindow;
}

// The slot function, that adds border after receiving signal
void bordering::border ()
{
    QDialog *dialog = new QDialog(MainWindow);
    Border_Dialog bdDialog;
    bdDialog.setupUi(dialog);
    if (dialog->exec() == QDialog::Accepted)
    {
    int penwidth = bdDialog.spinBox->value();

    QPainter painter(pm);
    QPen pen = QPen(Qt::black);
    pen.setWidth(penwidth);
    pen.setJoinStyle(Qt::MiterJoin);
    painter.setPen(pen);
    painter.drawRect(penwidth/2, penwidth/2, pm->width()-penwidth, pm->height()-penwidth);
    *pm_scaled = pm->scaledToHeight(*scale*pm->height(), Qt::SmoothTransformation);
    label->setPixmap(*pm_scaled);
    }
}


void Border_Dialog::setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
        Dialog->setObjectName("borderDialog");
        Dialog->resize(240, 141);
        Dialog->setModal(true);
        Dialog->setWindowTitle("Set Border Width");
        gridLayout = new QGridLayout(Dialog);

        buttonBox = new QDialogButtonBox(Dialog);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        gridLayout->addWidget(buttonBox, 4, 0, 1, 1);

//        QIntValidator *validator = new QIntValidator(Dialog);
        spinBox = new QSpinBox(Dialog);
        spinBox->setAlignment(Qt::AlignCenter);
        spinBox->setValue(5);
//        spinBox->setMaximumWidth(100);
        gridLayout->addWidget(spinBox, 2, 0, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        gridLayout->addItem(verticalSpacer, 3, 0, 1, 1);

        label = new QmyLabel(Dialog);
        label->setAlignment(Qt::AlignCenter);
        label->setText("Border Width");
        gridLayout->addWidget(label, 0, 0, 1, 1);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        gridLayout->addItem(verticalSpacer_2, 1, 0, 1, 1);



        QObject::connect(buttonBox, SIGNAL(accepted()), Dialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), Dialog, SLOT(reject()));

} // setupUi

