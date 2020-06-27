#include "invert.h"
#include <cmath>
// first parameter is name of plugin, usually same as the library file name
Q_EXPORT_PLUGIN2(invert, FilterPlugin);

void invert(QImage &img);

QString
FilterPlugin:: menuItem()
{
    // if you need / in menu name, use % character. Because / is path separator here
    return QString("Filter/Color/Invert%Negative");
}

void
FilterPlugin:: onMenuClick()
{
    invert(canvas->image);
    emit imageChanged();
}


//********* ---------- Invert Colors or Negate --------- ********** //
void invert(QImage &img)
{
    for (int y=0;y<img.height();y++) {
        QRgb* line = (QRgb*) img.scanLine(y);
        for (int x=0;x<img.width();x++) {
            line[x] = qRgba(255-qRed(line[x]), 255-qGreen(line[x]), 255-qBlue(line[x]), qAlpha(line[x]));
        }
    }
}


