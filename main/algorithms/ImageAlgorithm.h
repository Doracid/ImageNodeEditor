#pragma once

#include <QImage>
#include <QRect>
#include <QString>
#include <QColor>
#include <QtMath>

// Collection of stateless image processing functions.
// Each function takes input image(s), parameters, and returns the result.
// All functions are thread-safe (no mutable shared state).
class ImageAlgorithm {
public:
    // ---- Basic operations ----
    static QImage crop(const QImage &src, int x, int y, int w, int h);
    static QImage resize(const QImage &src, int w, int h, Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio);
    static QImage rotate(const QImage &src, double angleDeg, const QColor &bg = Qt::white);
    static QImage invert(const QImage &src);

    // ---- Color adjustment ----
    // brightness: -255..255, contrast: 0.0..3.0 (1.0 = none)
    static QImage brightnessContrast(const QImage &src, int brightness, double contrast);

    // ---- Filters ----
    static QImage gaussianBlur(const QImage &src, int radius);
    static QImage edgeDetection(const QImage &src, int lowThreshold, int highThreshold);

    // ---- Conversion ----
    static QImage toGrayscale(const QImage &src);

    // ---- Channel operations ----
    // Split returns 3 grayscale images for R, G, B respectively
    static void splitChannels(const QImage &src, QImage &r, QImage &g, QImage &b);
    // Merge combines 3 grayscale images into a color image
    static QImage mergeChannels(const QImage &r, const QImage &g, const QImage &b);

    // ---- Overlay ----
    static QImage addWatermark(const QImage &src, const QString &text,
                               int fontSize, const QColor &color, int opacity,
                               int posX, int posY);

private:
    // Helper: 1D Gaussian kernel weights
    static QVector<double> gaussianKernel(int radius);
    // Helper: clamp value to byte range
    static int clamp(int v) { return qBound(0, v, 255); }
};
