#ifndef CROPIMAGE_H
#define CROPIMAGE_H
#include <QLabel>
#include <QPushButton>
#include <QStatusBar>
#include <QPoint>
#include <QPixmap>
#include <QObject>
#include <QSpinBox>
#include <QCheckBox>

class cropper : public QObject
{
    Q_OBJECT
public:
    cropper(QLabel *Label, QStatusBar *Statusbar, QPixmap *Pm, QPixmap *Pm_scaled, float *Scale);
private:
    int height, width, dx,dy;
    bool clicked, cropEnabled, p1_dragable, p2_dragable;
    float *scale;
    QPoint p1, p2, pCursor;
    QPixmap *pm, *pm_scaled;
    QLabel *label, *colon, *wh;
    QPushButton *cropnowBtn, *cropcancelBtn;
    QStatusBar *statusbar;
    QSpinBox *widthratio, *heightratio;
    QCheckBox *lockratio;
    void drawBorder();
public slots:
    void cropperInit();
    void storeInit(QPoint);
    void storeFinal(QPoint);
    void drawBox(QPoint);
private slots:
    void cropImage();
    void cropCancel();
    void rotateleft();
    void rotateright();
    void changeLockMode(int);
};

#endif
