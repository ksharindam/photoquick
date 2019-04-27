#ifndef FILTERS_H
#define FILTERS_H

#include <QImage>

// Convert image to grayscale
void grayScale(QImage &img);

// Calculate otsu threshold value
int calcOtsuThresh(QImage img);

// Apply threshold for given global threshold value
void applyThresh(QImage &img, int thresh);

// Apply adaptive integral threshold using bradley's method
void adaptiveIntegralThresh(QImage &img);

#endif
