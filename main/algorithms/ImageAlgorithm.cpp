#include "ImageAlgorithm.h"
#include <QPainter>
#include <QTransform>
#include <QFont>
#include <cmath>
#include <algorithm>

// ====================================================================
// Crop
// ====================================================================
QImage ImageAlgorithm::crop(const QImage &src, int x, int y, int w, int h)
{
    if (src.isNull()) return {};
    QRect r(x, y, w, h);
    if (!src.rect().intersects(r)) return {};
    return src.copy(r.intersected(src.rect()));
}

// ====================================================================
// Resize
// ====================================================================
QImage ImageAlgorithm::resize(const QImage &src, int w, int h, Qt::AspectRatioMode mode)
{
    if (src.isNull()) return {};
    return src.scaled(w, h, mode, Qt::SmoothTransformation);
}

// ====================================================================
// Rotate
// ====================================================================
QImage ImageAlgorithm::rotate(const QImage &src, double angleDeg, const QColor &bg)
{
    if (src.isNull()) return {};
    QTransform tr;
    tr.rotate(angleDeg);
    QRect bounds = tr.mapRect(src.rect());
    QImage dst(bounds.size(), src.hasAlphaChannel() ? src.format() : QImage::Format_ARGB32);
    dst.fill(bg);
    QPainter p(&dst);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setTransform(tr);
    p.drawImage(bounds.topLeft() - src.offset(), src);
    p.end();
    return dst;
}

// ====================================================================
// Invert
// ====================================================================
QImage ImageAlgorithm::invert(const QImage &src)
{
    if (src.isNull()) return {};
    QImage dst = src.convertToFormat(QImage::Format_ARGB32);
    dst.invertPixels();
    return dst;
}

// ====================================================================
// Brightness / Contrast
// ====================================================================
QImage ImageAlgorithm::brightnessContrast(const QImage &src, int brightness, double contrast)
{
    if (src.isNull()) return {};
    QImage dst = src.convertToFormat(QImage::Format_ARGB32);

    // Precompute lookup table
    int table[256];
    for (int i = 0; i < 256; ++i) {
        double v = (i - 128) * contrast + 128 + brightness;
        table[i] = clamp((int)std::round(v));
    }

    for (int y = 0; y < dst.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < dst.width(); ++x) {
            QRgb px = line[x];
            int r = table[qRed(px)];
            int g = table[qGreen(px)];
            int b = table[qBlue(px)];
            line[x] = qRgba(r, g, b, qAlpha(px));
        }
    }
    return dst;
}

// ====================================================================
// Gaussian kernel
// ====================================================================
QVector<double> ImageAlgorithm::gaussianKernel(int radius)
{
    int size = 2 * radius + 1;
    QVector<double> kernel(size);
    double sigma = radius / 2.0;
    if (sigma < 0.5) sigma = 0.5;
    double sum = 0;
    for (int i = 0; i < size; ++i) {
        int x = i - radius;
        kernel[i] = std::exp(-(x * x) / (2 * sigma * sigma));
        sum += kernel[i];
    }
    for (auto &v : kernel) v /= sum;
    return kernel;
}

// ====================================================================
// Gaussian Blur (separable: horizontal + vertical)
// ====================================================================
QImage ImageAlgorithm::gaussianBlur(const QImage &src, int radius)
{
    if (src.isNull() || radius < 1) return src;
    QImage tmp = src.convertToFormat(QImage::Format_ARGB32);
    QImage dst(tmp.size(), tmp.format());

    auto kernel = gaussianKernel(radius);
    int ksize = kernel.size();

    // Horizontal pass (tmp -> tmp, column-major copy via row read)
    for (int y = 0; y < tmp.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(tmp.scanLine(y));
        QVector<QRgb> orig(line, line + tmp.width());
        for (int x = 0; x < tmp.width(); ++x) {
            double r = 0, g = 0, b = 0, a = 0;
            for (int k = 0; k < ksize; ++k) {
                int sx = qBound(0, x + k - radius, tmp.width() - 1);
                QRgb px = orig[sx];
                double w = kernel[k];
                r += qRed(px)   * w;
                g += qGreen(px) * w;
                b += qBlue(px)  * w;
                a += qAlpha(px) * w;
            }
            line[x] = qRgba(clamp((int)r), clamp((int)g), clamp((int)b), clamp((int)a));
        }
    }

    // Vertical pass (tmp -> dst)
    for (int x = 0; x < tmp.width(); ++x) {
        for (int y = 0; y < tmp.height(); ++y) {
            double r = 0, g = 0, b = 0, a = 0;
            for (int k = 0; k < ksize; ++k) {
                int sy = qBound(0, y + k - radius, tmp.height() - 1);
                QRgb px = reinterpret_cast<QRgb*>(tmp.scanLine(sy))[x];
                double w = kernel[k];
                r += qRed(px)   * w;
                g += qGreen(px) * w;
                b += qBlue(px)  * w;
                a += qAlpha(px) * w;
            }
            reinterpret_cast<QRgb*>(dst.scanLine(y))[x] =
                qRgba(clamp((int)r), clamp((int)g), clamp((int)b), clamp((int)a));
        }
    }

    return dst;
}

// ====================================================================
// Edge Detection (simple Sobel + threshold)
// ====================================================================
QImage ImageAlgorithm::edgeDetection(const QImage &src, int lowThreshold, int highThreshold)
{
    QImage gray = toGrayscale(src);
    if (gray.isNull()) return {};
    int w = gray.width(), h = gray.height();
    QImage edges(w, h, QImage::Format_Grayscale8);
    edges.fill(0);

    // Sobel kernels
    const int sobelX[3][3] = { {-1,0,1}, {-2,0,2}, {-1,0,1} };
    const int sobelY[3][3] = { {-1,-2,-1}, {0,0,0}, {1,2,1} };

    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            int gx = 0, gy = 0;
            for (int ky = -1; ky <= 1; ++ky) {
                const uchar *row = gray.scanLine(y + ky);
                for (int kx = -1; kx <= 1; ++kx) {
                    int val = row[x + kx];
                    gx += val * sobelX[ky + 1][kx + 1];
                    gy += val * sobelY[ky + 1][kx + 1];
                }
            }
            int mag = clamp((int)std::sqrt(gx * gx + gy * gy));
            // Double-threshold
            uchar out = (mag >= highThreshold) ? 255 : (mag >= lowThreshold ? 128 : 0);
            edges.scanLine(y)[x] = out;
        }
    }
    return edges;
}

// ====================================================================
// To Grayscale
// ====================================================================
QImage ImageAlgorithm::toGrayscale(const QImage &src)
{
    if (src.isNull()) return {};
    return src.convertToFormat(QImage::Format_Grayscale8);
}

// ====================================================================
// Split Channels
// ====================================================================
void ImageAlgorithm::splitChannels(const QImage &src, QImage &r, QImage &g, QImage &b)
{
    QImage argb = src.convertToFormat(QImage::Format_ARGB32);
    int w = argb.width(), h = argb.height();

    r = QImage(w, h, QImage::Format_Grayscale8);
    g = QImage(w, h, QImage::Format_Grayscale8);
    b = QImage(w, h, QImage::Format_Grayscale8);

    for (int y = 0; y < h; ++y) {
        const QRgb *srcLine = reinterpret_cast<const QRgb*>(argb.scanLine(y));
        uchar *rLine = r.scanLine(y);
        uchar *gLine = g.scanLine(y);
        uchar *bLine = b.scanLine(y);
        for (int x = 0; x < w; ++x) {
            QRgb px = srcLine[x];
            rLine[x] = qRed(px);
            gLine[x] = qGreen(px);
            bLine[x] = qBlue(px);
        }
    }
}

// ====================================================================
// Merge Channels
// ====================================================================
QImage ImageAlgorithm::mergeChannels(const QImage &r, const QImage &g, const QImage &b)
{
    int w = std::min({ r.width(), g.width(), b.width() });
    int h = std::min({ r.height(), g.height(), b.height() });
    if (w <= 0 || h <= 0) return {};

    QImage dst(w, h, QImage::Format_ARGB32);
    dst.fill(Qt::black);

    for (int y = 0; y < h; ++y) {
        QRgb *dstLine = reinterpret_cast<QRgb*>(dst.scanLine(y));
        const uchar *rLine = r.scanLine(y);
        const uchar *gLine = g.scanLine(y);
        const uchar *bLine = b.scanLine(y);
        for (int x = 0; x < w; ++x) {
            dstLine[x] = qRgba(rLine[x], gLine[x], bLine[x], 255);
        }
    }
    return dst;
}

// ====================================================================
// Watermark
// ====================================================================
QImage ImageAlgorithm::addWatermark(const QImage &src, const QString &text,
                                    int fontSize, const QColor &color, int opacity,
                                    int posX, int posY)
{
    if (src.isNull()) return {};
    QImage dst = src.convertToFormat(QImage::Format_ARGB32);

    QPainter p(&dst);
    p.setRenderHint(QPainter::Antialiasing);
    QFont font("Arial", fontSize);
    p.setFont(font);

    QColor c = color;
    c.setAlpha(qBound(0, opacity, 255));
    p.setPen(c);

    int x = src.width() / 2 + posX;
    int y = src.height() / 2 + posY;
    p.drawText(x, y, text);
    p.end();

    return dst;
}
