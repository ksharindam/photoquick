#ifndef FILEOPTIONS_H
#define FILEOPTIONS_H
#include <QMainWindow>
#include <QStatusBar>
#include <QLabel>
#include <QObject>
#include <QString>
#include <QVBoxLayout>
#include <QTimer>
#include <QIcon>
#include <QPushButton>

class fileOptions : public QObject
{
    Q_OBJECT
public:
    fileOptions(QMainWindow *, QStatusBar *, QLabel *, QPixmap *, 
                QPixmap *scaled, qreal *Scale, QVBoxLayout *, QPushButton *, int , int);
    void openfile(QString filename);
private:
    QMainWindow *MainWindow;
    QStatusBar *statusbar;
    QLabel *Label;
    qreal *scale;
    QPixmap *pm_scaled;
    QPixmap *pm;
    QString fileName;
    QVBoxLayout *vbox;
    QTimer *timer;
    bool slideshow_active;
    bool timer_created;
    QPushButton *slideshowButton;
    int offset_x;
    int offset_y;
public slots:
    void openImage();
    void saveImage();
    void openNext();
    void openPrev();
    void toggleSlideshow();
    void zoomIn();
    void zoomOut();
    void originalSize();
};
#endif
