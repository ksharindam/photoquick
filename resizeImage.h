#ifndef UI_RESIZEIMAGE_H
#define UI_RESIZEIMAGE_H

#include <QLabel>
#include <QObject>
#include <QMainWindow>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QSpacerItem>
#include <QStatusBar>
QT_BEGIN_NAMESPACE

class resizer : public QObject
{
    Q_OBJECT
public:
    resizer (QMainWindow *, QPixmap *, QPixmap *pixmap_scaled, float *Scale, QLabel *, QStatusBar *);
private:
    QMainWindow *MainWindow;
    QPixmap *pm;
    QPixmap *pm_scaled;
    float *scale;
    QLabel *label;
    QStatusBar *statusbar;

public slots:
    void resizeimage();
};


class ResizeDialog
{
public:
    QLineEdit *widthEdit;
    QLineEdit *heightEdit;

    void setupUi(QDialog *);
private:
    QGridLayout *gridLayout;
    QDialogButtonBox *buttonBox;
    QGridLayout *gridLayout_2;
    QSpacerItem *verticalSpacer_3;
    QLabel *label_2;
    QLabel *label;
};





QT_END_NAMESPACE

#endif // UI_RESIZEDIALOG_H
