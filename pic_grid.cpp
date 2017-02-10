// Image Viewer Main Source

#include "pic_grid.h"
#include "exif.h"
#include <QTransform>
#include <QFile>
#include <QIODevice>

//Subclassing of QFrame to provide multiple QLabel (Thumbnail) holding feature.

ImageHolder::ImageHolder(QWidget *parent) : QFrame(parent)
{
    this->setFrameShape(QFrame::StyledPanel);
    this->setFrameShadow(QFrame::Raised);
    verticalLayout = new QVBoxLayout(this);
}

void ImageHolder::addImage(QPixmap pixmap)
{
    Thumbnail *thumbnail = new Thumbnail(this);
    thumbnail->addPixmap(pixmap);
//    thumbnail->setAttribute(Qt::WA_DeleteOnClose);
    verticalLayout->addWidget(thumbnail);
    QObject::connect(thumbnail, SIGNAL(clicked(QPixmap)), this, SLOT(thumbnailClicked(QPixmap)));
}

void ImageHolder::addwidget(QWidget *widget)
{
    verticalLayout->addWidget(widget);
}

void ImageHolder::addspacer(QSpacerItem *verticalSpacer)
{
    verticalLayout->addItem(verticalSpacer);
}

void ImageHolder::loadImage()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                      "Select Image to Open", "/home/pi/Pictures",
                                      "Image files (*.jpg *.png *.jpeg *.bmp *.xpm *.ppm)");
    if (!filename.isEmpty()){
//        QPixmap pixmap = ;
      QPixmap pm = QPixmap(filename);

      int Orientation = 0;
      QFile file(filename);
      file.open(QIODevice::ReadOnly);
      Exif *exif = new Exif();
      exif->getExifOrientation(file, &Orientation);
      file.close();

      if ( Orientation == 6){
         QTransform transform;
         QTransform trans = transform.rotate(90);
         pm = pm.transformed(trans);}
      else if ( Orientation == 3){
         QTransform transform;
         QTransform trans = transform.rotate(180);
         pm = pm.transformed(trans);}
      addImage(pm);}
}


void ImageHolder::thumbnailClicked(QPixmap pixmap)
{
    emit clicked(pixmap);
}

/***************************************************************************/
// Subclass of QLabel that holds two pixmaps, one to display, another hidden

 Thumbnail::Thumbnail(QWidget *parent)
     : QLabel (parent)
 {
    originalPixmap = NULL;
 }

void Thumbnail::addPixmap(QPixmap pm){
    originalPixmap = pm;
    QPixmap pixmap = pm.scaledToWidth(100, Qt::SmoothTransformation);
    this->setPixmap(pixmap);
    this->setMaximumWidth(pixmap.width());
    this->setMaximumHeight(pixmap.height());
}

/* void Thumbnail::dragEnterEvent(QDragEnterEvent *event)
 {
     if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
         if (event->source() == this) {
             event->setDropAction(Qt::MoveAction);
             event->accept();
         } else {
             event->acceptProposedAction();
         }
     } else {
         event->ignore();
     }
 }
 void Thumbnail::dragMoveEvent(QDragMoveEvent *event)
 {
     if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
         if (event->source() == this) {
             event->setDropAction(Qt::MoveAction);
             event->accept();
         } else {
             event->acceptProposedAction();
         }
     } else {
         event->ignore();
     }
 }
*/
 void Thumbnail::mousePressEvent(QMouseEvent *event)
 {
/*     QLabel *child = static_cast<QLabel*>(childAt(event->pos()));
     if (!child)
         return;*/
     emit clicked(this->originalPixmap);
     QPixmap pixmap = *this->pixmap();
     QByteArray itemData;
     QDataStream dataStream(&itemData, QIODevice::WriteOnly);
     dataStream << pixmap << QPoint(event->pos());
     QMimeData *mimeData = new QMimeData;
     mimeData->setData("application/x-dnditemdata", itemData);
     QDrag *drag = new QDrag(this);
     drag->setMimeData(mimeData);
     drag->setPixmap(pixmap);
     drag->setHotSpot(event->pos());
     if (drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction) == Qt::MoveAction)
         this->close();
     else {
         this->show();
     }
 }

/***************************************************************************/
// Subclass of QLabel that has accept drops feature.

 ImageGrid::ImageGrid(QWidget *parent)
     : QLabel (parent)
 {
     setAcceptDrops(true);
     QPixmap pixmap = QPixmap(QSize(1800, 1200));
     pixmap.fill();
     setBackground(pixmap);
     img_width = 413;
     img_height = 531;
 }


void ImageGrid::setBackground(QPixmap pixmap){
    hidden_pixmap = pixmap;
    pg_width = pixmap.width();
    pg_height = pixmap.height();
    QPixmap qpm = pixmap.scaledToHeight(600, Qt::SmoothTransformation);
    scale = qreal(qpm.height())/qreal(pg_height);
    this->setPixmap(qpm);
    this->setMaximumWidth(qpm.width());
    this->setMaximumHeight(qpm.height());
}


void ImageGrid::setSourcePixmap(QPixmap pixmap){
    source_pixmap = pixmap;
}

static qreal nearest(QList<qreal> list, qreal comparator)
{
    for (int i=0;i<list.length();++i){
      qreal x = list[i];
      if (x< comparator)
        return x;
      }
    return list.last();
}

void ImageGrid::snapPixmap(int point_x, int point_y){
    QList<qreal> list_x;
    QList<qreal> list_y;
    qreal x, y;
    qreal spacing_x = (pg_width-4*img_width)/(4+1);
    qreal spacing_y = (pg_height-2*img_height)/(2+1);
    for (int i=1;i<5;++i){
      x = pg_width-i*(img_width+spacing_x);
      list_x.append(x);
    }
    for (int i=1;i<3;++i){
      y = pg_height-i*(img_height+spacing_y);
      list_y.append(y);
    }

    x = nearest(list_x, point_x/scale);
    y = nearest(list_y, point_y/scale);
    QPainter painter(&hidden_pixmap);
    painter.fillRect(QRect(x,y,img_width, img_height), Qt::white);
    painter.drawPixmap(x, y, source_pixmap.scaled(img_width, img_height, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    this->setPixmap(hidden_pixmap.scaledToHeight(600, Qt::SmoothTransformation));
    emit gridUpdated(hidden_pixmap);
}

 void ImageGrid::dragEnterEvent(QDragEnterEvent *event)
 {
     if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
         if (event->source() == this) {
             event->setDropAction(Qt::MoveAction);
             event->accept();
         } else {
             event->acceptProposedAction();
         }
     } else {
         event->ignore();
     }
 }
 void ImageGrid::dragMoveEvent(QDragMoveEvent *event)
 {
     if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
         if (event->source() == this) {
             event->setDropAction(Qt::MoveAction);
             event->accept();
         } else {
             event->acceptProposedAction();
         }
     } else {
         event->ignore();
     }
 }
 void ImageGrid::dropEvent(QDropEvent *event)
 {
     if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
         QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
         QDataStream dataStream(&itemData, QIODevice::ReadOnly);
         QPixmap pixmap;
         QPoint offset;
         dataStream >> pixmap >> offset;
         if (event->source() == this) {
             event->setDropAction(Qt::MoveAction);
             event->accept();
         } else {
             snapPixmap(event->pos().x(), event->pos().y());
             event->acceptProposedAction();
         }
     } else {
         event->ignore();
     }
 }

void Grid::Dialog::setGrid(QPixmap pixmap)
{
    image_grid = pixmap;
}

void Grid::Dialog::configUi(QPixmap image){
    frame->addImage(image);

    QObject::connect(frame, SIGNAL(clicked(QPixmap)), label_2, SLOT(setSourcePixmap(QPixmap)));
    QObject::connect(pushButton, SIGNAL(clicked()), frame, SLOT(loadImage()));
    QObject::connect(label_2, SIGNAL( gridUpdated(QPixmap) ), this, SLOT( setGrid(QPixmap) ));
//    dialog->show();

}


GridMaker::GridMaker(QMainWindow *MainWindow, QPixmap *Pixmap, QPixmap *Pixmap_scaled, float *Scale, QmyLabel *imagelabel)
{
    mainwindow = MainWindow;
    pixmap = Pixmap;
    pixmap_scaled = Pixmap_scaled;
    scale = Scale;
    label = imagelabel;
}

void GridMaker::createGrid()
{
    QDialog dialog(mainwindow);
    Grid::Dialog win;
    win.setupUi(&dialog);
    win.configUi(*pixmap);
    if (dialog.exec() == QDialog::Accepted){
        *pixmap = win.image_grid;
        *pixmap_scaled = *win.label_2->pixmap();
        *scale = qreal(pixmap_scaled->height())/ qreal(pixmap->height());
        label->setPixmap(*pixmap_scaled);
        mainwindow->resize(1100,700);
    }
}
