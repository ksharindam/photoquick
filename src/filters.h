#pragma once
#include <QImage>

// Convert image to grayscale
void grayScale(QImage &img);

// Calculate otsu threshold value
int calcOtsuThresh(QImage img);

// Apply threshold for given global threshold value
void threshold(QImage &img, int thresh);

// Apply adaptive integral threshold using bradley's method
void adaptiveThreshold(QImage &img);

// Apply Convolution Matrix
void convolve(QImage &img, float kernel[], int kernel_width);

// Gaussian Blur
void gaussianBlur(QImage &img, int radius=1, float sigma=0);

// Apply Box Blur
void boxFilter(QImage &img, int radius=1);

// Apply Median Filter
void medianFilter(QImage &img, int radius=1);

// Sharpen by Unsharp masking
void sharpen(QImage &img);

// Sigmoidal Contrast to enhance low light images
void sigmoidalContrast(QImage &img, float midpoint=0.5 /*0 to 1.0*/);

// Auto white balance
void autoWhiteBalance(QImage &img);

// remove speckle noise using crimmins speckle removal
void despeckle(QImage &img);

// Remove salt and pepper noise
//void reduceNoise(QImage &img, int radius=1);
