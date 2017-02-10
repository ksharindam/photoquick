#ifndef ADDBORDER_H
#define ADDBORDER_H

#include "QmyLabel.h"
#include <QPainter>
#include <QObject>
#include <QMainWindow>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QSpinBox>
#include <QtGui/QSpacerItem>

QT_BEGIN_NAMESPACE

class bordering : public QObject
{
    Q_OBJECT
public:
    bordering (QMainWindow *, QPixmap *, QPixmap *pixmap_scaled, float *Scale, QmyLabel *);
private:
    QMainWindow *MainWindow;
    QPixmap *pm;
    QPixmap *pm_scaled;
    float *scale;
    QmyLabel *label;
public slots:
    void border();
};


class Border_Dialog
{
public:
    QSpinBox *spinBox;
    void setupUi(QDialog *Dialog);
private:
    QGridLayout *gridLayout;
    QDialogButtonBox *buttonBox;
    QSpacerItem *verticalSpacer_2, *verticalSpacer;
    QmyLabel *label;
};



QT_END_NAMESPACE

#endif
