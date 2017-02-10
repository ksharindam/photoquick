// Image Viewer Main Source

#include "ui_mainwindow.h"
#include "pic_grid.h"

Ui::MainWindow::~MainWindow(){
    fopt->deleteLater();
    resimg->deleteLater();
    cropimg->deleteLater();
    delete scale;
    borderadd->deleteLater();
}

void Ui::MainWindow::setupConfig(QMainWindow *MainWindow)
{
    win = MainWindow;
    scale = new qreal;
    *scale = 1.0;
    settings = new QSettings("Qmage", "qmage", win);
    int offset_x = settings->value("OffsetX", 4).toInt();
    int offset_y = settings->value("OffsetY", 26).toInt();
    fopt = new fileOptions(MainWindow,statusbar, label, &pm, &pm_scaled,
                           scale, verticalLayout, slideshowButton, offset_x, offset_y);
    resimg = new resizer(MainWindow, &pm, &pm_scaled, scale, label, statusbar);
    cropimg = new cropper(label, statusbar, &pm, &pm_scaled, scale);
    borderadd = new bordering(MainWindow, &pm, &pm_scaled, scale, label);
    GridMaker *gridmaker = new GridMaker(MainWindow, &pm, &pm_scaled, scale, label);

    QObject::connect(openButton, SIGNAL(clicked()), fopt, SLOT(openImage()));
    QObject::connect(saveButton, SIGNAL(clicked()), fopt, SLOT(saveImage()));
    QObject::connect(nextButton, SIGNAL(clicked()), fopt, SLOT(openNext()));
    QObject::connect(prevButton, SIGNAL(clicked()), fopt, SLOT(openPrev()));
    QObject::connect(slideshowButton, SIGNAL(clicked()), fopt, SLOT(toggleSlideshow()));
    QObject::connect(zoomButton, SIGNAL(clicked()), fopt, SLOT(zoomIn()));
    QObject::connect(zoomoutButton, SIGNAL(clicked()), fopt, SLOT(zoomOut()));
    QObject::connect(originalsizeButton, SIGNAL(clicked()), fopt, SLOT(originalSize()));
    QObject::connect(rotateleftButton, SIGNAL(clicked()), cropimg, SLOT(rotateleft()));
    QObject::connect(rotaterightButton, SIGNAL(clicked()), cropimg, SLOT(rotateright()));
    QObject::connect(resizeButton, SIGNAL(clicked()), resimg, SLOT(resizeimage()));

    QObject::connect(cropButton, SIGNAL(clicked()), cropimg, SLOT(cropperInit()));

    QObject::connect(borderButton, SIGNAL(clicked()), borderadd, SLOT(border()));
    QObject::connect(gridButton, SIGNAL(clicked()), gridmaker, SLOT(createGrid()));

    QObject::connect(label, SIGNAL(mousePressed(QPoint)), cropimg, SLOT(storeInit(QPoint)));
    QObject::connect(label, SIGNAL(mouseMoved(QPoint)), cropimg, SLOT(drawBox(QPoint)));
    QObject::connect(label, SIGNAL(mouseReleased(QPoint)), cropimg, SLOT(storeFinal(QPoint)));

    MainWindow->show();

}

void MyWindow::closeEvent(QCloseEvent *event)
{
    QSettings *settings = new QSettings("Qmage-View", "qmage-view", this);
    settings->setValue("OffsetX", geometry().x()-x());
    settings->setValue("OffsetY", geometry().y()-y()); 
    return QMainWindow::closeEvent(event);
}

int main(int argc, char *argv[])
{
    QString filename;
    if (argc < 2 || argc > 3)
      filename = QString::fromUtf8(":/nidhi.jpg");
    else 
      filename = QString::fromUtf8(argv[1]);

    QApplication app(argc, argv);
    MyWindow rootwin;
    Ui::MainWindow win;
    win.setupUi(&rootwin);
    win.setupConfig(&rootwin);
    win.fopt->openfile(filename);
    return app.exec();
}
