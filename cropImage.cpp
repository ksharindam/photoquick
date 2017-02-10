#include "cropImage.h"
#include <QPainter>
#include <iostream>
#include <QString>
#include <QTransform>
#include <QColor>

// Create required variables here
cropper::cropper(QLabel *Label, QStatusBar *Statusbar, QPixmap *Pm, QPixmap *Pm_scaled, float *Scale )
{
    label = Label;
    statusbar = Statusbar;
    pm = Pm;
    pm_scaled = Pm_scaled;
    scale = Scale;
    cropEnabled = false;
    p1_dragable = false;
    p2_dragable = false;
}

void cropper::cropperInit ()
{

    clicked = false;
    width = pm_scaled->width();
    height = pm_scaled->height();
    p1 = QPoint(0,0);              // point of top-left corner
    p2 = QPoint(width-1,height-1);// point of bottom-right corner

    lockratio = new QCheckBox(statusbar);
    lockratio->setText("Lock Ratio  ");
    statusbar->addPermanentWidget(lockratio);
    QObject::connect(lockratio, SIGNAL(stateChanged(int)), this, SLOT(changeLockMode(int)));
    wh = new QLabel(statusbar);
    wh->setText("<b>W:H =</b>");
    statusbar->addPermanentWidget(wh);
    wh->setEnabled(false);
    widthratio = new QSpinBox(statusbar);
    widthratio->setMinimum(1);
    widthratio->setMaximum(25);
    widthratio->setMaximumWidth(32);
    widthratio->setValue(7);
    widthratio->setEnabled(false);
    statusbar->addPermanentWidget(widthratio);
    colon = new QLabel(statusbar);
    colon->setText("<b>:</b>");
    statusbar->addPermanentWidget(colon);
    heightratio = new QSpinBox(statusbar);
    heightratio->setMinimum(1);
    heightratio->setMaximum(25);
    heightratio->setMaximumWidth(32);
    heightratio->setValue(9);
    heightratio->setEnabled(false);
    statusbar->addPermanentWidget(heightratio);
    cropnowBtn = new QPushButton("Crop Now");
    statusbar->addPermanentWidget(cropnowBtn);
    QObject::connect(cropnowBtn, SIGNAL(clicked()), this, SLOT(cropImage()));
    cropcancelBtn = new QPushButton("Cancel");
    statusbar->addPermanentWidget(cropcancelBtn);
    QObject::connect(cropcancelBtn, SIGNAL(clicked()), this, SLOT(cropCancel()));
    cropEnabled = true;
    drawBorder();
}

void cropper::cropImage()
{
    *pm = pm->copy((p1.x()/ *scale),(p1.y()/ *scale), (p2.x()-p1.x()+1)/ *scale, (p2.y()-p1.y()+1)/ *scale);
    *pm_scaled = pm->scaledToHeight(*scale*pm->height(), Qt::SmoothTransformation);
    label->setPixmap(*pm_scaled);
    statusbar->removeWidget(widthratio);
    statusbar->removeWidget(heightratio);
    statusbar->removeWidget(colon);
    statusbar->removeWidget(lockratio);
    statusbar->removeWidget(wh);
    statusbar->removeWidget(cropnowBtn);
    statusbar->removeWidget(cropcancelBtn);
     lockratio->deleteLater();
     colon->deleteLater();
     heightratio->deleteLater();
     widthratio->deleteLater();
     wh->deleteLater();
     cropnowBtn->deleteLater();
     cropcancelBtn->deleteLater();
    cropEnabled = false;
    QString qs = QString("Resolution : %1x%2 , Scale : %3x").arg(pm->width()).arg(pm->height()).arg(*scale);
    statusbar->showMessage(qs);
}

void cropper::storeInit(QPoint p)
{
    if (cropEnabled) {
      clicked = true;
      pCursor = QPoint(p.x(), p.y());
      if (pCursor.x()>=p1.x() && pCursor.x()<=p1.x()+60 && pCursor.y()>=p1.y() && pCursor.y()<=p1.y()+60)
        p1_dragable = true;
      else
        p1_dragable = false;
      if (pCursor.x()<=p2.x() && pCursor.x()>=p2.x()-60 && pCursor.y()<=p2.y() && pCursor.y()>=p2.y()-60)
        p2_dragable = true;
      else
        p2_dragable = false;
//      std::cout << qs.toStdString() << std::endl;
    }
}

void cropper::storeFinal(QPoint)
{
    if (cropEnabled) {
      clicked = false;
    }
}

void cropper::drawBox(QPoint p)
{
    if (cropEnabled) {
     if (clicked){
        dx = p.x()-pCursor.x();
        dy = p.y()-pCursor.y();
        qreal imgaspect = qreal(widthratio->value())/qreal(heightratio->value());

     if (p.x()>= 0 && p.x()<= width-1 && p.y()>= 0 && p.y()<= height-1)
     { // cursor is inside image

      if (p1_dragable) { // top-left corner is dragged
        if (p1.x() <= (-dx) && dx < 0 )
          dx = -p1.x();
          
        if (p1.y() <= -dy && dy < 0) // dy is -ve, and cursor tends to go ouside the image
          dy = -p1.y();
          
        p1 = QPoint( p1.x()+dx, p1.y()+dy );
        // When Ratio is locked, adjust bottom-right corner position when top-left is moved.
        if (lockratio->checkState() != 0){
            if (imgaspect > qreal(p2.x()-p1.x()+1)/qreal(p2.y()-p1.y()+1))
              p1 = QPoint( p1.x(), p2.y()-(p2.x()-p1.x())/imgaspect );
            else
              p1 = QPoint( p2.x()-(p2.y()-p1.y())*imgaspect, p1.y() );}
      }

      else if (p2_dragable) { // bottom-right corner is dragged
        if (width-p2.x()-1 <=dx && dx > 0)
          dx = width-p2.x()-1;
        if (height-p2.y()-1 <= dy && dy > 0)
          dy = height-p2.y()-1;

        p2 = QPoint( p2.x()+dx, p2.y()+dy );
        // When Ratio is locked, adjust top-left corner position when bottom-right is moved.
        if (lockratio->checkState() != 0){
            if (imgaspect > qreal(p2.x()-p1.x()+1)/qreal(p2.y()-p1.y()+1))
              p2 = QPoint(p2.x(), p1.y()+(p2.x()-p1.x())/imgaspect);
            else
              p2 = QPoint(p1.x()+(p2.y()-p1.y())*imgaspect, p2.y());}
      }
      else { // When the box is dragged
        if (p1.x() <= (-dx) && dx < 0 )
          dx = -p1.x();
          
        if (p1.y() <= -dy && dy < 0)        // dy is -ve, and cursor tends to go ouside the image
          dy = -p1.y();
          
        if (width-p2.x()-1 <=dx && dx > 0)
          dx = width-1-p2.x();
        if (height-p2.y()-1 <= dy && dy > 0)
          dy = height-1-p2.y();

        p1 = QPoint( p1.x()+dx, p1.y()+dy );
        p2 = QPoint( p2.x()+dx, p2.y()+dy );}
        }
          
        // Draw Rectangle
        pCursor = p;
        drawBorder();
        QString qs = QString("Resolution : %1x%2").arg((p2.x()-p1.x()+1)/ *scale).arg((p2.y()-p1.y()+1)/ *scale);
        statusbar->showMessage(qs);
      }}
}
void cropper::drawBorder()
{
    QPixmap qpm = pm_scaled->copy();
    QPixmap pm_box = pm_scaled->copy(p1.x(), p1.y(),p2.x()-p1.x(), p2.y()-p1.y());
    QPainter painter(&qpm);
    painter.fillRect(0,0,qpm.width(),qpm.height(), QColor(127,127,127,127));
    painter.drawPixmap(p1.x(), p1.y(), pm_box);
    painter.setPen(QPen(Qt::white));
    painter.drawRect(p1.x()+1, p1.y()+1,p2.x()-p1.x()-2, p2.y()-p1.y()-2);
    painter.setPen(QPen(Qt::black));
    painter.drawRect(p1.x(), p1.y(),p2.x()-p1.x(), p2.y()-p1.y());
    painter.drawRect(p1.x(), p1.y() ,60,60);
    painter.drawRect(p2.x()-60, p2.y()-60 ,60,60);
    label->setPixmap(qpm);

}

void cropper::changeLockMode(int state){ // Toggles Lock Ratio Mode
    if (state == 0){
      wh->setEnabled(false);
      widthratio->setEnabled(false);
      heightratio->setEnabled(false);}
    else {
      wh->setEnabled(true);
      widthratio->setEnabled(true);
      heightratio->setEnabled(true);}
}

void cropper::cropCancel(){
    label->setPixmap(*pm_scaled);
    cropEnabled = false;
    statusbar->removeWidget(widthratio);
    statusbar->removeWidget(heightratio);
    statusbar->removeWidget(colon);
    statusbar->removeWidget(lockratio);
    statusbar->removeWidget(wh);
    statusbar->removeWidget(cropnowBtn);
    statusbar->removeWidget(cropcancelBtn);
    statusbar->clearMessage();
    lockratio->deleteLater();
    colon->deleteLater();
    heightratio->deleteLater();
    widthratio->deleteLater();
    wh->deleteLater();
    cropnowBtn->deleteLater();
    cropcancelBtn->deleteLater();
}

void cropper::rotateleft(){
    QTransform transform;
    QTransform trans = transform.rotate(270);
    *pm = pm->transformed(trans);
    *pm_scaled = pm->scaledToHeight(*scale*pm->height(), Qt::SmoothTransformation);
    label->setPixmap(*pm_scaled);
    QString qs = QString("Resolution : %1x%2 , Scale : %3x").arg(pm->width()).arg(pm->height()).arg(*scale);
    statusbar->showMessage(qs);
}

void cropper::rotateright(){
    QTransform transform;
    QTransform trans = transform.rotate(90);
    *pm = pm->transformed(trans);
    *pm_scaled = pm->scaledToHeight(*scale*pm->height(), Qt::SmoothTransformation);
    label->setPixmap(*pm_scaled);
    QString qs = QString("Resolution : %1x%2 , Scale : %3x").arg(pm->width()).arg(pm->height()).arg(*scale);
    statusbar->showMessage(qs);
}
