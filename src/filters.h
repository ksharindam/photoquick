#ifndef FILTERS_H
#define FILTERS_H

#include <QImage>

// Convert image to grayscale
void grayScale(QImage &img);

// Calculate otsu threshold value
int calcOtsuThresh(QImage img);

// Apply threshold for given global threshold value
void globalThresh(QImage &img, int thresh);

// Apply adaptive integral threshold using bradley's method
void adaptiveIntegralThresh(QImage &img);

// Apply Box Blur
void boxBlur(QImage &img, int radius=1);

#endif
