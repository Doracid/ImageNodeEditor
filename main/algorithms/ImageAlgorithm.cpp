#include "ImageAlgorithm.h"
#include <QPainter>
#include <QTransform>
#include <QFont>
#include <QRandomGenerator>
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
// Rotate — around image center, output is minimum bounding rect
// ====================================================================
QImage ImageAlgorithm::rotate(const QImage &src, double angleDeg, const QColor &bg)
{
    if (src.isNull()) return {};
    double cx = src.width() / 2.0;
    double cy = src.height() / 2.0;
    double rad = angleDeg * M_PI / 180.0;
    double cosA = std::cos(rad);
    double sinA = std::sin(rad);

    // Compute bounding box of the 4 corners rotated around center
    // Corners relative to center: (-cx,-cy), (w-cx,-cy), (w-cx,h-cy), (-cx,h-cy)
    double rx[4], ry[4];
    double cxr[4] = {-cx, src.width() - cx, src.width() - cx, -cx};
    double cyr[4] = {-cy, -cy, src.height() - cy, src.height() - cy};
    double minX = 1e18, minY = 1e18, maxX = -1e18, maxY = -1e18;
    for (int i = 0; i < 4; ++i) {
        rx[i] = cxr[i] * cosA - cyr[i] * sinA;
        ry[i] = cxr[i] * sinA + cyr[i] * cosA;
        if (rx[i] < minX) minX = rx[i];
        if (rx[i] > maxX) maxX = rx[i];
        if (ry[i] < minY) minY = ry[i];
        if (ry[i] > maxY) maxY = ry[i];
    }

    int outW = (int)std::ceil(maxX - minX);
    int outH = (int)std::ceil(maxY - minY);

    QImage dst(outW, outH, QImage::Format_ARGB32);
    dst.fill(bg);

    // Paint: output center → rotate → draw source centered
    QPainter p(&dst);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(outW / 2.0, outH / 2.0);
    p.rotate(angleDeg);
    p.drawImage(QPointF(-cx, -cy), src);
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
// Watermark — centered, with rotation support
// ====================================================================
QImage ImageAlgorithm::addWatermark(const QImage &src, const QString &text,
                                    int fontSize, const QColor &color, int opacity,
                                    int posX, int posY, double rotation)
{
    if (src.isNull()) return {};
    QImage dst = src.convertToFormat(QImage::Format_ARGB32);

    QPainter p(&dst);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    QFont font("Arial", fontSize);
    p.setFont(font);

    QColor c = color;
    c.setAlpha(qBound(0, opacity, 255));
    p.setPen(c);

    QFontMetrics fm(font);
    int tw = fm.horizontalAdvance(text);
    int th = fm.height();

    // Center of image + user offset
    int cx = src.width() / 2 + posX;
    int cy = src.height() / 2 + posY;

    QRect textRect(-tw / 2, -th / 2, tw, th);
    if (qAbs(rotation) > 0.001) {
        p.save();
        p.translate(cx, cy);
        p.rotate(rotation);
        p.drawText(textRect, Qt::AlignCenter, text);
        p.restore();
    } else {
        p.save();
        p.translate(cx, cy);
        p.drawText(textRect, Qt::AlignCenter, text);
        p.restore();
    }
    p.end();

    return dst;
}

// ====================================================================
// HSL helpers — range: h[0,360), s[0,1], l[0,1]
// ====================================================================
static void rgbToHsl(int r, int g, int b, double &h, double &s, double &l)
{
    double rd = r / 255.0, gd = g / 255.0, bd = b / 255.0;
    double mx = std::max({rd, gd, bd});
    double mn = std::min({rd, gd, bd});
    l = (mx + mn) / 2.0;
    if (mx == mn) { h = 0; s = 0; return; }
    double d = mx - mn;
    s = (l > 0.5) ? d / (2.0 - mx - mn) : d / (mx + mn);
    if (mx == rd)       h = 60.0 * fmod((gd - bd) / d, 6.0);
    else if (mx == gd)  h = 60.0 * ((bd - rd) / d + 2.0);
    else                h = 60.0 * ((rd - gd) / d + 4.0);
    if (h < 0) h += 360.0;
}

static int hueToRgb(double p, double q, double t)
{
    if (t < 0) t += 1.0;
    if (t > 1) t -= 1.0;
    if (t < 1.0/6.0) return (int)std::round((p + (q - p) * 6.0 * t) * 255.0);
    if (t < 1.0/2.0) return (int)std::round(q * 255.0);
    if (t < 2.0/3.0) return (int)std::round((p + (q - p) * (2.0/3.0 - t) * 6.0) * 255.0);
    return (int)std::round(p * 255.0);
}

static void hslToRgb(double h, double s, double l, int &r, int &g, int &b)
{
    if (s == 0) { r = g = b = (int)std::round(l * 255.0); return; }
    double q = (l < 0.5) ? l * (1.0 + s) : l + s - l * s;
    double p = 2.0 * l - q;
    double hd = h / 360.0;
    r = hueToRgb(p, q, hd + 1.0/3.0);
    g = hueToRgb(p, q, hd);
    b = hueToRgb(p, q, hd - 1.0/3.0);
}

// ====================================================================
// Saturation
// ====================================================================
QImage ImageAlgorithm::saturation(const QImage &src, double factor)
{
    if (src.isNull()) return {};
    QImage dst = src.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < dst.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < dst.width(); ++x) {
            QRgb px = line[x];
            int r = qRed(px), g = qGreen(px), b = qBlue(px);
            double h, s, l;
            rgbToHsl(r, g, b, h, s, l);
            s = qBound(0.0, s * factor, 1.0);
            hslToRgb(h, s, l, r, g, b);
            line[x] = qRgba(r, g, b, qAlpha(px));
        }
    }
    return dst;
}

// ====================================================================
// Hue Shift
// ====================================================================
QImage ImageAlgorithm::hueShift(const QImage &src, int angle)
{
    if (src.isNull()) return {};
    QImage dst = src.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < dst.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < dst.width(); ++x) {
            QRgb px = line[x];
            int r = qRed(px), g = qGreen(px), b = qBlue(px);
            double h, s, l;
            rgbToHsl(r, g, b, h, s, l);
            h = fmod(h + angle, 360.0);
            if (h < 0) h += 360.0;
            hslToRgb(h, s, l, r, g, b);
            line[x] = qRgba(r, g, b, qAlpha(px));
        }
    }
    return dst;
}

// ====================================================================
// Gamma Correction
// ====================================================================
QImage ImageAlgorithm::gammaCorrection(const QImage &src, double gamma)
{
    if (src.isNull() || gamma <= 0.0) return {};
    double inv = 1.0 / gamma;
    int table[256];
    for (int i = 0; i < 256; ++i)
        table[i] = clamp((int)std::round(std::pow(i / 255.0, inv) * 255.0));

    QImage dst = src.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < dst.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < dst.width(); ++x) {
            QRgb px = line[x];
            line[x] = qRgba(table[qRed(px)], table[qGreen(px)], table[qBlue(px)], qAlpha(px));
        }
    }
    return dst;
}

// ====================================================================
// Threshold
// ====================================================================
QImage ImageAlgorithm::threshold(const QImage &src, int level)
{
    if (src.isNull()) return {};
    QImage gray = toGrayscale(src);
    QImage dst(gray.size(), QImage::Format_Grayscale8);
    for (int y = 0; y < gray.height(); ++y) {
        const uchar *srcLine = gray.scanLine(y);
        uchar *dstLine = dst.scanLine(y);
        for (int x = 0; x < gray.width(); ++x)
            dstLine[x] = (srcLine[x] >= level) ? 255 : 0;
    }
    return dst;
}

// ====================================================================
// Binarize (multi-mode)
// ====================================================================
QImage ImageAlgorithm::binarize(const QImage &src, int mode, int level,
                                int blockSize, double c)
{
    if (src.isNull()) return {};
    QImage gray = toGrayscale(src);
    int w = gray.width(), h = gray.height();

    if (mode == 0) {
        // Global threshold
        return threshold(src, level);
    }

    if (mode == 1) {
        // OTSU: find threshold that maximizes between-class variance
        qint64 hist[256] = {0};
        for (int y = 0; y < h; ++y) {
            const uchar *line = gray.scanLine(y);
            for (int x = 0; x < w; ++x)
                hist[line[x]]++;
        }
        qint64 total = w * h;
        qint64 sum = 0;
        for (int i = 0; i < 256; ++i) sum += i * hist[i];
        qint64 sumB = 0, wB = 0;
        double maxVar = 0;
        int otsuLevel = 0;
        for (int i = 0; i < 256; ++i) {
            wB += hist[i];
            if (wB == 0) continue;
            qint64 wF = total - wB;
            if (wF == 0) break;
            sumB += i * hist[i];
            double meanB = (double)sumB / wB;
            double meanF = (double)(sum - sumB) / wF;
            double var = (double)wB * wF * (meanB - meanF) * (meanB - meanF);
            if (var > maxVar) { maxVar = var; otsuLevel = i; }
        }
        QImage dst(gray.size(), QImage::Format_Grayscale8);
        for (int y = 0; y < h; ++y) {
            const uchar *srcLine = gray.scanLine(y);
            uchar *dstLine = dst.scanLine(y);
            for (int x = 0; x < w; ++x)
                dstLine[x] = (srcLine[x] >= otsuLevel) ? 255 : 0;
        }
        return dst;
    }

    // Adaptive modes
    if (blockSize < 3) blockSize = 3;
    if (blockSize % 2 == 0) blockSize++;
    int half = blockSize / 2;

    if (mode == 2) {
        // Adaptive mean: threshold = local mean - c
        QImage dst(gray.size(), QImage::Format_Grayscale8);
        for (int y = 0; y < h; ++y) {
            const uchar *srcLine = gray.scanLine(y);
            uchar *dstLine = dst.scanLine(y);
            for (int x = 0; x < w; ++x) {
                int sum = 0, count = 0;
                for (int dy = -half; dy <= half; ++dy) {
                    int ny = qBound(0, y + dy, h - 1);
                    const uchar *row = gray.scanLine(ny);
                    for (int dx = -half; dx <= half; ++dx) {
                        int nx = qBound(0, x + dx, w - 1);
                        sum += row[nx];
                        count++;
                    }
                }
                double mean = (double)sum / count;
                dstLine[x] = (srcLine[x] >= mean - c) ? 255 : 0;
            }
        }
        return dst;
    }

    if (mode == 3) {
        // Adaptive gaussian: weighted local mean - c using gaussian kernel
        // Precompute gaussian kernel
        QVector<double> kernel(blockSize);
        double sigma = blockSize / 6.0;
        double sumK = 0;
        for (int i = 0; i < blockSize; ++i) {
            double dx = i - half;
            kernel[i] = qExp(-(dx * dx) / (2.0 * sigma * sigma));
            sumK += kernel[i];
        }
        for (int i = 0; i < blockSize; ++i) kernel[i] /= sumK;

        QImage dst(gray.size(), QImage::Format_Grayscale8);
        for (int y = 0; y < h; ++y) {
            const uchar *srcLine = gray.scanLine(y);
            uchar *dstLine = dst.scanLine(y);
            for (int x = 0; x < w; ++x) {
                double weightedSum = 0, weightTotal = 0;
                for (int dy = -half; dy <= half; ++dy) {
                    int ny = qBound(0, y + dy, h - 1);
                    const uchar *row = gray.scanLine(ny);
                    double ky = kernel[dy + half];
                    for (int dx = -half; dx <= half; ++dx) {
                        int nx = qBound(0, x + dx, w - 1);
                        double kx = kernel[dx + half];
                        weightedSum += ky * kx * row[nx];
                        weightTotal += ky * kx;
                    }
                }
                double mean = weightedSum / weightTotal;
                dstLine[x] = (srcLine[x] >= mean - c) ? 255 : 0;
            }
        }
        return dst;
    }

    // Fallback to global
    return threshold(src, level);
}

// ====================================================================
// Sepia
// ====================================================================
QImage ImageAlgorithm::sepia(const QImage &src, double intensity)
{
    if (src.isNull()) return {};
    QImage dst = src.convertToFormat(QImage::Format_ARGB32);
    double t = qBound(0.0, intensity, 1.0);
    for (int y = 0; y < dst.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < dst.width(); ++x) {
            QRgb px = line[x];
            int r = qRed(px), g = qGreen(px), b = qBlue(px);
            int sr = clamp((int)(r * 0.393 + g * 0.769 + b * 0.189));
            int sg = clamp((int)(r * 0.349 + g * 0.686 + b * 0.168));
            int sb = clamp((int)(r * 0.272 + g * 0.534 + b * 0.131));
            // Blend with original
            line[x] = qRgba(
                clamp((int)(sr * t + r * (1 - t))),
                clamp((int)(sg * t + g * (1 - t))),
                clamp((int)(sb * t + b * (1 - t))),
                qAlpha(px));
        }
    }
    return dst;
}

// ====================================================================
// Sharpen (3x3 kernel)
// ====================================================================
QImage ImageAlgorithm::sharpen(const QImage &src, double strength)
{
    if (src.isNull()) return {};
    QImage tmp = src.convertToFormat(QImage::Format_ARGB32);
    QImage dst(tmp.size(), tmp.format());
    double k = qBound(0.0, strength, 10.0);
    // kernel: -k  -k  -k
    //        -k 1+8k -k
    //        -k  -k  -k
    for (int y = 0; y < tmp.height(); ++y) {
        for (int x = 0; x < tmp.width(); ++x) {
            QRgb c = reinterpret_cast<QRgb*>(tmp.scanLine(y))[x];
            double r = qRed(c)   * (1.0 + 8.0 * k);
            double g = qGreen(c) * (1.0 + 8.0 * k);
            double b = qBlue(c)  * (1.0 + 8.0 * k);
            for (int ky = -1; ky <= 1; ++ky) {
                int sy = qBound(0, y + ky, tmp.height() - 1);
                for (int kx = -1; kx <= 1; ++kx) {
                    if (kx == 0 && ky == 0) continue;
                    QRgb p = reinterpret_cast<QRgb*>(tmp.scanLine(sy))[qBound(0, x + kx, tmp.width() - 1)];
                    r -= k * qRed(p);   g -= k * qGreen(p);   b -= k * qBlue(p);
                }
            }
            int ri = qBound(0, (int)std::round(r), 255);
            int gi = qBound(0, (int)std::round(g), 255);
            int bi = qBound(0, (int)std::round(b), 255);
            reinterpret_cast<QRgb*>(dst.scanLine(y))[x] = qRgba(ri, gi, bi, qAlpha(c));
        }
    }
    return dst;
}

// ====================================================================
// Color Temperature — channel gain in RGB
// ====================================================================
QImage ImageAlgorithm::colorTemperature(const QImage &src, double temperature)
{
    if (src.isNull()) return {};
    double t = qBound(-1.0, temperature, 1.0);
    double rGain = 1.0 + t * 0.3;
    double bGain = 1.0 - t * 0.3;
    double gGain = 1.0 + std::abs(t) * 0.08;

    QImage dst = src.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < dst.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < dst.width(); ++x) {
            QRgb px = line[x];
            line[x] = qRgba(
                clamp((int)std::round(qRed(px)   * rGain)),
                clamp((int)std::round(qGreen(px) * gGain)),
                clamp((int)std::round(qBlue(px)  * bGain)),
                qAlpha(px));
        }
    }
    return dst;
}

// ====================================================================
// Fade — lerp to mid-gray
// ====================================================================
QImage ImageAlgorithm::fade(const QImage &src, double fade)
{
    if (src.isNull()) return {};
    double f = qBound(0.0, fade, 1.0);
    QImage dst = src.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < dst.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < dst.width(); ++x) {
            QRgb px = line[x];
            int r = qRed(px), g = qGreen(px), b = qBlue(px);
            int gray = (r + g + b) / 3;
            line[x] = qRgba(
                clamp((int)(r + (gray - r) * f)),
                clamp((int)(g + (gray - g) * f)),
                clamp((int)(b + (gray - b) * f)),
                qAlpha(px));
        }
    }
    return dst;
}

// ====================================================================
// Pixelate — block down-sample + nearest-neighbor up-sample
// ====================================================================
QImage ImageAlgorithm::pixelate(const QImage &src, int blockSize)
{
    if (src.isNull()) return {};
    int bs = qMax(2, blockSize);
    QImage dst = src.convertToFormat(QImage::Format_ARGB32);
    int w = dst.width(), h = dst.height();

    for (int by = 0; by < h; by += bs) {
        for (int bx = 0; bx < w; bx += bs) {
            QRgb color = reinterpret_cast<QRgb*>(dst.scanLine(by))[bx];
            int ex = qMin(bx + bs, w);
            int ey = qMin(by + bs, h);
            for (int y = by; y < ey; ++y) {
                QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
                for (int x = bx; x < ex; ++x)
                    line[x] = color;
            }
        }
    }
    return dst;
}

// ====================================================================
// Pixel Art — pixelation with per-pixel shape styles and optional outline
// shapeMode: 0=square, 1=rounded square, 2=circle
// showOutline: true = draw black border around each pixel
// bgWhite: true=white background, false=black background
// ====================================================================
QImage ImageAlgorithm::pixelArt(const QImage &src, int blockSize, int shapeMode, bool showOutline, int outlineWidth, bool bgWhite)
{
    if (src.isNull()) return {};
    int bs = qMax(2, blockSize);
    QImage input = src.convertToFormat(QImage::Format_ARGB32);
    int w = input.width(), h = input.height();

    QImage dst(w, h, QImage::Format_ARGB32);
    QColor bgColor = bgWhite ? Qt::white : Qt::black;
    dst.fill(bgColor);

    // Pre-compute block colors (top-left pixel of each block)
    struct BlockColor { int r, g, b; };
    int cols = (w + bs - 1) / bs;
    int rows = (h + bs - 1) / bs;
    QVector<QVector<BlockColor>> blockColors(rows, QVector<BlockColor>(cols));
    for (int by = 0; by < rows; ++by) {
        for (int bx = 0; bx < cols; ++bx) {
            int sx = bx * bs, sy = by * bs;
            QRgb color = reinterpret_cast<QRgb*>(input.scanLine(sy))[sx];
            blockColors[by][bx] = { qRed(color), qGreen(color), qBlue(color) };
        }
    }

    // Square + no outline → fast path: just fill blocks like pixelate
    if (shapeMode == 0 && !showOutline) {
        for (int by = 0; by < rows; ++by) {
            for (int bx = 0; bx < cols; ++bx) {
                int x = bx * bs, y = by * bs;
                int bw = qMin(bs, w - x);
                int bh = qMin(bs, h - y);
                auto &c = blockColors[by][bx];
                QRgb color = qRgba(c.r, c.g, c.b, 255);
                for (int dy = 0; dy < bh; ++dy) {
                    QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y + dy));
                    for (int dx = 0; dx < bw; ++dx)
                        line[x + dx] = color;
                }
            }
        }
        return dst;
    }

    // Use QPainter for shape drawing
    QPainter p(&dst);
    p.setRenderHint(QPainter::Antialiasing);

    QPen outlinePen(Qt::black, qMax(1, outlineWidth));

    for (int by = 0; by < rows; ++by) {
        for (int bx = 0; bx < cols; ++bx) {
            int x = bx * bs, y = by * bs;
            int bw = qMin(bs, w - x);
            int bh = qMin(bs, h - y);
            auto &c = blockColors[by][bx];
            QColor fillColor(c.r, c.g, c.b);

            const int margin = qMax(1, bs / 16) + (showOutline ? outlineWidth : 0);
            int sx = x + margin;
            int sy = y + margin;
            int sw = bw - 2 * margin;
            int sh = bh - 2 * margin;
            if (sw < 1 || sh < 1) {
                p.fillRect(x, y, bw, bh, fillColor);
                continue;
            }

            p.setBrush(fillColor);
            if (showOutline)
                p.setPen(outlinePen);
            else
                p.setPen(Qt::NoPen);

            switch (shapeMode) {
            case 0: // Square
                p.drawRect(sx, sy, sw, sh);
                break;
            case 1: { // Rounded square
                int cornerR = qMin(sw, sh) / 4;
                p.drawRoundedRect(sx, sy, sw, sh, cornerR, cornerR);
                break;
            }
            case 2: { // Circle
                int cx = x + bw / 2;
                int cy = y + bh / 2;
                int radius = qMin(bw, bh) / 2 - margin;
                if (radius > 0)
                    p.drawEllipse(QPoint(cx, cy), radius, radius);
                break;
            }
            }
        }
    }
    p.end();
    return dst;
}

// ====================================================================
// Vignette — radial falloff from center
// ====================================================================
QImage ImageAlgorithm::vignette(const QImage &src, double radius, double strength)
{
    if (src.isNull()) return {};
    double r = qBound(0.0, radius, 1.0);
    double s = qBound(0.0, strength, 5.0);
    QImage dst = src.convertToFormat(QImage::Format_ARGB32);
    int w = dst.width(), h = dst.height();
    double cx = w / 2.0, cy = h / 2.0;

    for (int y = 0; y < h; ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
        double dy = (y - cy) / cy;
        for (int x = 0; x < w; ++x) {
            double dx = (x - cx) / cx;
            double dist = std::sqrt(dx * dx + dy * dy);
            double falloff = 1.0 - s * qMax(dist - r, 0.0);
            falloff = qBound(0.0, falloff, 1.0);
            QRgb px = line[x];
            line[x] = qRgba(
                clamp((int)std::round(qRed(px)   * falloff)),
                clamp((int)std::round(qGreen(px) * falloff)),
                clamp((int)std::round(qBlue(px)  * falloff)),
                qAlpha(px));
        }
    }
    return dst;
}

// ====================================================================
// Pencil Sketch — grayscale + invert-blur + dodge blend
// enhanced with paper grain texture and cross-hatching
// ====================================================================

// Generate paper-like grain noise (0..255, 0 = smooth, 255 = rough)
static QImage generatePaperGrain(int w, int h, int seed)
{
    QImage grain(w, h, QImage::Format_Grayscale8);
    QRandomGenerator rng(seed);
    // White noise
    for (int y = 0; y < h; ++y) {
        uchar *line = grain.scanLine(y);
        for (int x = 0; x < w; ++x)
            line[x] = (uchar)(rng.bounded(256));
    }

    // Blur to make it organic (small kernel = paper fiber feel)
    QImage blurred = ImageAlgorithm::gaussianBlur(grain, 2);
    // Stretch contrast to restore range after blur
    for (int y = 0; y < h; ++y) {
        uchar *line = blurred.scanLine(y);
        for (int x = 0; x < w; ++x) {
            int v = line[x];
            v = (v - 96) * 4;  // stretch around mid-gray
            line[x] = (uchar)qBound(0, v, 255);
        }
    }
    return blurred;
}

// Generate cross-hatching pattern: diagonal lines, density mapped to tone (0=black,255=white)
static QImage generateHatching(int w, int h, const QImage &toneRef)
{
    QImage hatch(w, h, QImage::Format_Grayscale8);
    hatch.fill(255); // start white

    for (int y = 0; y < h; ++y) {
        const uchar *tLine = toneRef.scanLine(y);
        uchar *hLine = hatch.scanLine(y);
        for (int x = 0; x < w; ++x) {
            int tone = tLine[x];           // 0=black(shadow), 255=white(highlight)
            // Line density: darker = more lines
            double density = 1.0 - tone / 255.0;  // 0..1
            if (density < 0.08) continue;  // skip near-white areas

            // Check two diagonal directions (45° and 135°)
            int phase1 = (x - y) & 63;     // 45° diagonal spacing = 64 pixels
            int phase2 = (x + y) & 63;     // 135° diagonal spacing = 64 pixels
            double lineWidth = density * 3.0;

            bool onLine = (phase1 < lineWidth) || (phase2 < lineWidth);
            if (onLine) {
                int darken = (int)((1.0 - density) * 220 + density * 250);
                hLine[x] = (uchar)qBound(0, darken, 255);
            }
        }
    }
    // Soften hatching slightly
    hatch = ImageAlgorithm::gaussianBlur(hatch, 1);
    return hatch;
}

// Histogram equalization for Grayscale8 image (detail boost)
static QImage histogramEqualize(const QImage &gray, double strength)
{
    if (gray.isNull() || strength <= 0.0) return gray;
    int total = gray.width() * gray.height();
    if (total == 0) return gray;

    int hist[256] = {0};
    for (int y = 0; y < gray.height(); ++y) {
        const uchar *line = gray.scanLine(y);
        for (int x = 0; x < gray.width(); ++x)
            hist[line[x]]++;
    }

    int cdf[256]; cdf[0] = hist[0];
    for (int i = 1; i < 256; ++i) cdf[i] = cdf[i - 1] + hist[i];
    int cdfMin = 0;
    for (int i = 0; i < 256; ++i) { if (cdf[i] > 0) { cdfMin = cdf[i]; break; } }

    double scale = 255.0 / (total - cdfMin);
    int lut[256];
    for (int i = 0; i < 256; ++i)
        lut[i] = qBound(0, (int)std::round((cdf[i] - cdfMin) * scale), 255);

    if (strength >= 1.0) {
        QImage result(gray.size(), QImage::Format_Grayscale8);
        for (int y = 0; y < gray.height(); ++y) {
            const uchar *s = gray.scanLine(y);
            uchar *d = result.scanLine(y);
            for (int x = 0; x < gray.width(); ++x)
                d[x] = (uchar)lut[s[x]];
        }
        return result;
    }
    QImage result(gray.size(), QImage::Format_Grayscale8);
    for (int y = 0; y < gray.height(); ++y) {
        const uchar *s = gray.scanLine(y);
        uchar *d = result.scanLine(y);
        for (int x = 0; x < gray.width(); ++x) {
            int eq = lut[s[x]];
            d[x] = (uchar)qBound(0, (int)(s[x] * (1.0 - strength) + eq * strength), 255);
        }
    }
    return result;
}

QImage ImageAlgorithm::pencilSketch(const QImage &src, int blurRadius, double detailBoost)
{
    if (src.isNull()) return {};
    int rad = qBound(1, blurRadius, 15);
    double db = qBound(0.0, detailBoost, 1.0);
    int w = src.width(), h = src.height();
    if (w > 2000 || h > 2000) {
        QImage small = src.scaled(2000, 2000, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QImage result = pencilSketch(small, blurRadius, detailBoost);
        return result.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    QImage gray = toGrayscale(src);

    // Histogram equalization to preserve highlight/shadow details
    QImage eq = histogramEqualize(gray, db);
    if (db > 0.0 && db < 1.0) {
        for (int y = 0; y < gray.height(); ++y) {
            const uchar *s = gray.scanLine(y);
            const uchar *e = eq.scanLine(y);
            uchar *d = eq.scanLine(y);
            for (int x = 0; x < gray.width(); ++x)
                d[x] = (uchar)(s[x] * (1.0 - db) + e[x] * db);
        }
    }
    gray = (db > 0.0) ? eq : gray;
    QImage inv(gray.size(), QImage::Format_Grayscale8);
    for (int y = 0; y < gray.height(); ++y) {
        const uchar *s = gray.scanLine(y);
        uchar *d = inv.scanLine(y);
        for (int x = 0; x < gray.width(); ++x)
            d[x] = 255 - s[x];
    }

    QImage blurred = gaussianBlur(inv, rad);
    gray = gray.convertToFormat(QImage::Format_ARGB32);
    blurred = blurred.convertToFormat(QImage::Format_ARGB32);

    QImage dst(gray.size(), QImage::Format_ARGB32);
    // Dodge-blend to get base sketch
    {
        QImage gray8 = gray.convertToFormat(QImage::Format_Grayscale8);
        QImage blr8   = blurred.convertToFormat(QImage::Format_Grayscale8);
        for (int y = 0; y < gray.height(); ++y) {
            const uchar *gLine = gray8.constScanLine(y);
            const uchar *bLine = blr8.constScanLine(y);
            QRgb *dLine = reinterpret_cast<QRgb*>(dst.scanLine(y));
            for (int x = 0; x < gray.width(); ++x) {
                int gv = gLine[x];
                int bv = bLine[x];
                int sketch = gv * 255 / qMax(bv + 1, 1);
                sketch = qMin(sketch, 255);
                dLine[x] = qRgba(sketch, sketch, sketch, 255);
            }
        }
    }

    // Sobel edge overlay — captures fine details even in low-contrast areas
    {
        QImage gray8 = toGrayscale(src); // use original (not equalized) for edges
        int w = gray8.width(), h = gray8.height();
        // Compute edge magnitude and darken sketch at edge pixels
        for (int y = 1; y < h - 1; ++y) {
            const uchar *row0 = gray8.constScanLine(y - 1);
            const uchar *row1 = gray8.constScanLine(y);
            const uchar *row2 = gray8.constScanLine(y + 1);
            QRgb *dLine = reinterpret_cast<QRgb*>(dst.scanLine(y));
            for (int x = 1; x < w - 1; ++x) {
                int gx = row0[x-1] + 2*row0[x] + row0[x+1]
                       - row2[x-1] - 2*row2[x] - row2[x+1];
                int gy = row0[x-1] + 2*row1[x-1] + row2[x-1]
                       - row0[x+1] - 2*row1[x+1] - row2[x+1];
                int mag = (int)std::sqrt(gx * gx + gy * gy);
                if (mag > 8) { // threshold to avoid noise
                    int sketch = qRed(dLine[x]);
                    // Darken: stronger edge = more darkening
                    double edgeStr = qMin(1.0, mag / 80.0);
                    int darken = (int)(sketch * (1.0 - edgeStr * 0.55));
                    dLine[x] = qRgba(darken, darken, darken, 255);
                }
            }
        }
    }

    // Generate and blend paper grain texture
    QImage grain = generatePaperGrain(w, h, 42);
    QImage gray8 = dst.convertToFormat(QImage::Format_Grayscale8);
    QImage hatch = generateHatching(w, h, gray8);

    // Composite grain + hatching onto sketch
    // Coefficients: keep sketch as base, overlay grain and hatch
    for (int y = 0; y < h; ++y) {
        QRgb *dLine = reinterpret_cast<QRgb*>(dst.scanLine(y));
        uchar *gLine = grain.scanLine(y);
        uchar *hLine = hatch.scanLine(y);
        for (int x = 0; x < w; ++x) {
            int sketch = qRed(dLine[x]);
            int grainV = gLine[x];
            int hatchV = hLine[x];

            // Combine: grain affects mid-tones more
            double sketchN = sketch / 255.0;
            double grainN  = (grainV - 128) / 128.0;  // -1..+1

            // Grain modulated by mid-tone prominence (most visible on 50% gray)
            double grainFactor = 1.0 - std::abs(sketchN - 0.5) * 2.0;
            grainFactor = qMax(0.0, grainFactor) * 0.25;

            int v = (int)(sketch + grainN * grainFactor * 255.0);

            // Hatching overlay: multiply (darkens)
            double hatchFactor = hatchV / 255.0;
            v = (int)(v * (0.7 + 0.3 * hatchFactor));

            v = qBound(0, v, 255);
            dLine[x] = qRgba(v, v, v, 255);
        }
    }

    return dst;
}

// ====================================================================
// Cartoon — color posterize + edge overlay
// ====================================================================
QImage ImageAlgorithm::cartoon(const QImage &src, int edgeThreshold, int levels)
{
    if (src.isNull()) return {};
    int lv = qBound(2, levels, 16);
    int step = 255 / lv;
    int thr = qBound(0, edgeThreshold, 255);

    QImage dst = src.convertToFormat(QImage::Format_ARGB32);
    int w = dst.width(), h = dst.height();

    // 1. Posterize color levels
    for (int y = 0; y < h; ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < w; ++x) {
            QRgb px = line[x];
            int r = qRed(px), g = qGreen(px), b = qBlue(px);
            r = qMin(r / step * step + step / 2, 255);
            g = qMin(g / step * step + step / 2, 255);
            b = qMin(b / step * step + step / 2, 255);
            line[x] = qRgba(r, g, b, qAlpha(px));
        }
    }

    // 2. Sobel edge detection on original grayscale → overlay black edges
    QImage gray = toGrayscale(src);
    for (int y = 1; y < h - 1; ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 1; x < w - 1; ++x) {
            int gx = 0, gy = 0;
            for (int ky = -1; ky <= 1; ++ky) {
                const uchar *row = gray.constScanLine(y + ky);
                for (int kx = -1; kx <= 1; ++kx) {
                    int val = row[x + kx];
                    static const int sobelX[3] = {-1, 0, 1};
                    static const int sobelY[3] = {-1, 0, 1};
                    gx += val * sobelX[kx + 1] * (ky == 0 ? 2 : 1);
                    gy += val * sobelY[ky + 1] * (kx == 0 ? 2 : 1);
                }
            }
            int mag = qBound(0, (int)std::round(std::sqrt(gx * gx + gy * gy)), 255);
            if (mag > thr)
                line[x] = qRgba(0, 0, 0, qAlpha(line[x]));
        }
    }
    return dst;
}

// ====================================================================
// Comic Style — black outlines on white background
// ====================================================================
QImage ImageAlgorithm::comicStyle(const QImage &src, int edgeThreshold, int lineThickness)
{
    if (src.isNull()) return {};
    int thr = qBound(8, edgeThreshold, 255);
    int thick = qBound(1, lineThickness, 5);
    int h = src.height(), w = src.width();

    QImage gray = toGrayscale(src);

    // Dark fill threshold: pixels darker than this are filled black
    const int fillThr = 64;

    // Step 1: Sobel edge detection + dark fill classification
    QImage output(w, h, QImage::Format_ARGB32);
    output.fill(Qt::white);

    // Classify each pixel: 0=light, 1=dark-fill, 2=edge(black), 3=edge(white)
    // We'll use a temporary grayscale image to store classification
    QImage classMap(w, h, QImage::Format_Grayscale8);
    classMap.fill(0);

    // First pass: dark fill and edge detection
    for (int y = 1; y < h - 1; ++y) {
        const uchar *row0 = gray.constScanLine(y - 1);
        const uchar *row1 = gray.constScanLine(y);
        const uchar *row2 = gray.constScanLine(y + 1);
        uchar *cLine = classMap.scanLine(y);
        for (int x = 1; x < w - 1; ++x) {
            int gx = row0[x-1] + 2*row0[x] + row0[x+1]
                   - row2[x-1] - 2*row2[x] - row2[x+1];
            int gy = row0[x-1] + 2*row1[x-1] + row2[x-1]
                   - row0[x+1] - 2*row1[x+1] - row2[x+1];
            int mag = (int)std::sqrt(gx * gx + gy * gy);

            if (mag >= thr) {
                // Edge pixel: check if it separates two dark areas
                int darkCount = 0;
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        if (dx == 0 && dy == 0) continue;
                        int ny = y + dy, nx = x + dx;
                        if (ny >= 0 && ny < h && nx >= 0 && nx < w) {
                            if (gray.constScanLine(ny)[nx] < fillThr)
                                darkCount++;
                        }
                    }
                }
                // If most neighbors are dark → white separation line
                // Otherwise → black outline
                cLine[x] = (darkCount >= 5) ? 3 : 2;
            } else if (row1[x] < fillThr) {
                // Non-edge dark pixel → black fill
                cLine[x] = 1;
            }
        }
    }

    // Handle border pixels (simple copy from nearest classified pixel)
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (x == 0 || x == w-1 || y == 0 || y == h-1) {
                int cx = qBound(1, x, w-2);
                int cy = qBound(1, y, h-2);
                uchar val = classMap.constScanLine(cy)[cx];
                classMap.scanLine(y)[x] = val;
            }
        }
    }

    // Dilate edges (both black and white edges)
    if (thick > 1) {
        QImage dilated(w, h, QImage::Format_Grayscale8);
        dilated.fill(0);
        int rad = thick;
        for (int y = rad; y < h - rad; ++y) {
            for (int x = rad; x < w - rad; ++x) {
                uchar v = classMap.constScanLine(y)[x];
                if (v >= 2) { // edge pixel
                    for (int dy = -rad; dy <= rad; ++dy)
                        for (int dx = -rad; dx <= rad; ++dx)
                            if (dx*dx + dy*dy <= rad*rad)
                                if (dy + y >= 0 && dy + y < h && dx + x >= 0 && dx + x < w)
                                    if (classMap.constScanLine(y + dy)[x + dx] != 1)
                                        dilated.scanLine(y + dy)[x + dx] = v;
                }
            }
        }
        // Copy back: only update non-dark-fill pixels
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                uchar dv = dilated.constScanLine(y)[x];
                if (dv >= 2 && classMap.constScanLine(y)[x] != 1)
                    classMap.scanLine(y)[x] = dv;
            }
        }
    }

    // Render output
    for (int y = 0; y < h; ++y) {
        const uchar *cLine = classMap.constScanLine(y);
        QRgb *dLine = reinterpret_cast<QRgb*>(output.scanLine(y));
        for (int x = 0; x < w; ++x) {
            switch (cLine[x]) {
                case 1:  dLine[x] = qRgba(0, 0, 0, 255); break;     // black fill
                case 2:  dLine[x] = qRgba(0, 0, 0, 255); break;     // black edge
                case 3:  dLine[x] = qRgba(255, 255, 255, 255); break; // white gap
                default: /* white background */ break;
            }
        }
    }
    return output;
}

// ====================================================================
// Gradient Map — map grayscale luminance through a color gradient
// ====================================================================
QImage ImageAlgorithm::gradientMap(const QImage &src, const QVector<QPair<double, QColor>> &stops)
{
    if (src.isNull()) return {};
    if (stops.size() < 2) return src;

    // Build 256-entry color LUT from gradient stops
    // Sort stops by position
    QVector<QPair<double, QColor>> sorted = stops;
    std::sort(sorted.begin(), sorted.end(),
              [](const QPair<double, QColor> &a, const QPair<double, QColor> &b) {
                  return a.first < b.first;
              });

    QRgb lut[256];
    for (int i = 0; i < 256; ++i) {
        double t = i / 255.0;

        // Find the two stops that bracket t
        if (t <= sorted[0].first) {
            lut[i] = sorted[0].second.rgba();
        } else if (t >= sorted.last().first) {
            lut[i] = sorted.last().second.rgba();
        } else {
            for (int s = 0; s < sorted.size() - 1; ++s) {
                if (t >= sorted[s].first && t <= sorted[s + 1].first) {
                    double local = (t - sorted[s].first)
                                 / (sorted[s + 1].first - sorted[s].first);
                    const QColor &c1 = sorted[s].second;
                    const QColor &c2 = sorted[s + 1].second;
                    lut[i] = qRgba(
                        (int)(c1.red()   * (1.0 - local) + c2.red()   * local),
                        (int)(c1.green() * (1.0 - local) + c2.green() * local),
                        (int)(c1.blue()  * (1.0 - local) + c2.blue()  * local),
                        255);
                    break;
                }
            }
        }
    }

    // Apply LUT based on grayscale luminance
    QImage gray = toGrayscale(src);
    int w = gray.width(), h = gray.height();
    QImage dst(w, h, QImage::Format_ARGB32);
    for (int y = 0; y < h; ++y) {
        const uchar *gLine = gray.constScanLine(y);
        QRgb *dLine = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < w; ++x) {
            dLine[x] = lut[gLine[x]];
        }
    }
    return dst;
}

// ====================================================================
// Exposure — out = in * 2^EV
// ====================================================================
QImage ImageAlgorithm::exposure(const QImage &src, double ev)
{
    if (src.isNull()) return {};
    double factor = std::pow(2.0, qBound(-5.0, ev, 5.0));
    int table[256];
    for (int i = 0; i < 256; ++i)
        table[i] = clamp((int)std::round(i * factor));

    QImage dst = src.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < dst.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < dst.width(); ++x) {
            QRgb px = line[x];
            line[x] = qRgba(table[qRed(px)], table[qGreen(px)], table[qBlue(px)], qAlpha(px));
        }
    }
    return dst;
}

// ====================================================================
// Vibrance — smart saturation boost
// ====================================================================
QImage ImageAlgorithm::vibrance(const QImage &src, double amount)
{
    if (src.isNull()) return {};
    double a = qBound(0.0, amount, 1.0);
    QImage dst = src.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < dst.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < dst.width(); ++x) {
            QRgb px = line[x];
            int r = qRed(px), g = qGreen(px), b = qBlue(px);
            double h, s, l;
            rgbToHsl(r, g, b, h, s, l);

            // More boost for low-saturation pixels, protection for saturated areas
            double gain = a * (1.0 - s);
            s = qBound(0.0, s + s * gain, 1.0);

            hslToRgb(h, s, l, r, g, b);
            line[x] = qRgba(r, g, b, qAlpha(px));
        }
    }
    return dst;
}

// ====================================================================
// White Balance — RGB channel gains for temperature & tint
// ====================================================================
QImage ImageAlgorithm::whiteBalance(const QImage &src, double temperature, double tint)
{
    if (src.isNull()) return {};
    double t = qBound(-1.0, temperature, 1.0);
    double tn = qBound(-1.0, tint, 1.0);

    // Temperature: R vs B gain (warm → boost R, cut B; cool → cut R, boost B)
    double rGain = 1.0 + t * 0.25;
    double bGain = 1.0 - t * 0.25;
    // Tint: G vs (R+B)/2 (green↔magenta)
    double gGain = 1.0 + tn * 0.15;
    double rTint = 1.0 - tn * 0.05;
    double bTint = 1.0 + tn * 0.05;

    QImage dst = src.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < dst.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < dst.width(); ++x) {
            QRgb px = line[x];
            line[x] = qRgba(
                clamp((int)std::round(qRed(px)   * rGain * rTint)),
                clamp((int)std::round(qGreen(px) * gGain)),
                clamp((int)std::round(qBlue(px)  * bGain * bTint)),
                qAlpha(px));
        }
    }
    return dst;
}

// ====================================================================
// Auto White Balance — gray-world assumption
// ====================================================================
QImage ImageAlgorithm::autoWhiteBalance(const QImage &src)
{
    if (src.isNull()) return {};
    QImage argb = src.convertToFormat(QImage::Format_ARGB32);
    int w = argb.width(), h = argb.height();
    int total = w * h;
    if (total == 0) return {};

    double sumR = 0, sumG = 0, sumB = 0;
    for (int y = 0; y < h; ++y) {
        const QRgb *line = reinterpret_cast<const QRgb*>(argb.constScanLine(y));
        for (int x = 0; x < w; ++x) {
            QRgb px = line[x];
            sumR += qRed(px);
            sumG += qGreen(px);
            sumB += qBlue(px);
        }
    }
    double avgR = sumR / total;
    double avgG = sumG / total;
    double avgB = sumB / total;
    double target = (avgR + avgG + avgB) / 3.0;

    double rGain = (avgR > 1.0) ? target / avgR : 1.0;
    double gGain = (avgG > 1.0) ? target / avgG : 1.0;
    double bGain = (avgB > 1.0) ? target / avgB : 1.0;

    QImage dst = src.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < dst.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < dst.width(); ++x) {
            QRgb px = line[x];
            line[x] = qRgba(
                clamp((int)std::round(qRed(px)   * rGain)),
                clamp((int)std::round(qGreen(px) * gGain)),
                clamp((int)std::round(qBlue(px)  * bGain)),
                qAlpha(px));
        }
    }
    return dst;
}

// ====================================================================
// Highlights / Shadows — luminance-mask-based adjustment
// ====================================================================
QImage ImageAlgorithm::highlightsShadows(const QImage &src, double highlights, double shadows)
{
    if (src.isNull()) return {};
    double hAmt = qBound(-1.0, highlights, 1.0);
    double sAmt = qBound(-1.0, shadows, 1.0);
    if (qAbs(hAmt) < 0.005 && qAbs(sAmt) < 0.005) return src;

    QImage dst = src.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < dst.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < dst.width(); ++x) {
            QRgb px = line[x];
            int r = qRed(px), g = qGreen(px), b = qBlue(px);
            double l = (0.299 * r + 0.587 * g + 0.114 * b) / 255.0;

            // Highlights mask: cubic — strong for bright pixels
            double hMask = l * l * l;
            double hAdj = hAmt * hMask;

            // Shadows mask: cubic — strong for dark pixels
            double sMask = (1.0 - l) * (1.0 - l) * (1.0 - l);
            double sAdj = sAmt * sMask;

            double totalAdj = hAdj + sAdj;
            double factor = 1.0 + totalAdj;
            r = clamp((int)std::round(r * factor));
            g = clamp((int)std::round(g * factor));
            b = clamp((int)std::round(b * factor));
            line[x] = qRgba(r, g, b, qAlpha(px));
        }
    }
    return dst;
}

// ====================================================================
// Whites / Blacks — endpoint pushing via piecewise LUT
// ====================================================================
QImage ImageAlgorithm::whitesBlacks(const QImage &src, double whites, double blacks)
{
    if (src.isNull()) return {};
    double w = qBound(0.0, whites, 1.0);
    double b = qBound(0.0, blacks, 1.0);
    if (w < 0.005 && b < 0.005) return src;

    int table[256];
    for (int i = 0; i < 256; ++i) {
        double v = i / 255.0;
        // Blacks: push lower portion toward 0
        if (b > 0.0 && v < 0.5) {
            double t = v / 0.5;
            v = v * (1.0 - b * t);
        }
        // Whites: push upper portion toward 1
        if (w > 0.0 && v > 0.5) {
            double t = (v - 0.5) / 0.5;
            v = v + w * t * (1.0 - v);
        }
        table[i] = clamp((int)std::round(v * 255.0));
    }

    QImage dst = src.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < dst.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < dst.width(); ++x) {
            QRgb px = line[x];
            line[x] = qRgba(table[qRed(px)], table[qGreen(px)], table[qBlue(px)], qAlpha(px));
        }
    }
    return dst;
}

// ====================================================================
// Clarity — mid-frequency detail enhancement
// ====================================================================
QImage ImageAlgorithm::clarity(const QImage &src, double amount, int radius)
{
    if (src.isNull()) return {};
    double a = qBound(0.0, amount, 1.0);
    int rad = qBound(1, radius, 30);
    if (a < 0.005) return src;

    QImage blurred = gaussianBlur(src, rad);
    if (blurred.isNull()) return src;

    QImage srcArgb = src.convertToFormat(QImage::Format_ARGB32);
    QImage blrArgb = blurred.convertToFormat(QImage::Format_ARGB32);
    QImage dst(srcArgb.size(), QImage::Format_ARGB32);

    for (int y = 0; y < srcArgb.height(); ++y) {
        QRgb *sLine = reinterpret_cast<QRgb*>(srcArgb.scanLine(y));
        QRgb *bLine = reinterpret_cast<QRgb*>(blrArgb.scanLine(y));
        QRgb *dLine = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < srcArgb.width(); ++x) {
            QRgb spx = sLine[x];
            QRgb bpx = bLine[x];
            int r = qRed(spx),    g = qGreen(spx),    b = qBlue(spx);
            int br = qRed(bpx),   bg = qGreen(bpx),   bb = qBlue(bpx);
            // detail = original - blur
            int dr = r - br, dg = g - bg, db = b - bb;
            // out = original + detail * amount
            r = clamp((int)std::round(r + dr * a));
            g = clamp((int)std::round(g + dg * a));
            b = clamp((int)std::round(b + db * a));
            dLine[x] = qRgba(r, g, b, qAlpha(spx));
        }
    }
    return dst;
}

// ====================================================================
// Tone Curve — 256-level LUT with linear interpolation
// ====================================================================
static QVector<int> buildCurveLUT(const QVector<QPointF> &points)
{
    QVector<int> lut(256);
    if (points.size() < 2) {
        for (int i = 0; i < 256; ++i) lut[i] = i;
        return lut;
    }

    QVector<QPoint> pts;
    for (const auto &p : points)
        pts.append(QPoint(qBound(0, (int)std::round(p.x() * 255.0), 255),
                          qBound(0, (int)std::round(p.y() * 255.0), 255)));

    // Ensure endpoints for complete coverage
    if (pts.first().x() > 0) pts.prepend(QPoint(0, 0));
    if (pts.last().x() < 255) pts.append(QPoint(255, 255));

    for (int i = 0; i < pts.size() - 1; ++i) {
        int x0 = pts[i].x(), y0 = pts[i].y();
        int x1 = pts[i + 1].x(), y1 = pts[i + 1].y();
        int dx = x1 - x0;
        int dy = y1 - y0;
        for (int x = x0; x <= x1; ++x) {
            double t = (dx == 0) ? 0.0 : (double)(x - x0) / dx;
            lut[x] = qBound(0, (int)std::round(y0 + dy * t), 255);
        }
    }
    return lut;
}

QImage ImageAlgorithm::toneCurve(const QImage &src, const QVector<QPointF> &points, bool /*mono*/)
{
    if (src.isNull()) return {};
    QVector<int> lut = buildCurveLUT(points);

    QImage dst = src.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < dst.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < dst.width(); ++x) {
            QRgb px = line[x];
            line[x] = qRgba(lut[qRed(px)], lut[qGreen(px)], lut[qBlue(px)], qAlpha(px));
        }
    }
    return dst;
}

// ====================================================================
// Auto Levels — stretch luminance histogram to fill full range
// ====================================================================
QImage ImageAlgorithm::autoLevels(const QImage &src, double clipPercent)
{
    if (src.isNull()) return {};
    double clip = qBound(0.001, clipPercent, 5.0);
    QImage argb = src.convertToFormat(QImage::Format_ARGB32);
    int w = argb.width(), h = argb.height();
    int total = w * h;
    if (total == 0) return {};

    // Build luminance histogram
    int hist[256] = {0};
    for (int y = 0; y < h; ++y) {
        const QRgb *line = reinterpret_cast<const QRgb*>(argb.constScanLine(y));
        for (int x = 0; x < w; ++x) {
            QRgb px = line[x];
            int lum = qGray(px);
            hist[lum]++;
        }
    }

    // Find low percentile
    int clipCount = (int)(total * clip / 100.0);
    int lowVal = 0, sum = 0;
    for (int i = 0; i < 256; ++i) {
        sum += hist[i];
        if (sum > clipCount) { lowVal = i; break; }
    }

    // Find high percentile
    int highVal = 255;
    sum = 0;
    for (int i = 255; i >= 0; --i) {
        sum += hist[i];
        if (sum > clipCount) { highVal = i; break; }
    }

    if (highVal <= lowVal) return argb;

    // Build LUT: [lowVal, highVal] → [0, 255]
    int lut[256];
    double range = highVal - lowVal;
    for (int i = 0; i < 256; ++i) {
        if (i <= lowVal)         lut[i] = 0;
        else if (i >= highVal)   lut[i] = 255;
        else                     lut[i] = (int)((i - lowVal) / range * 255.0);
    }

    // Apply same LUT to all channels (preserves color balance)
    QImage dst(argb.size(), QImage::Format_ARGB32);
    for (int y = 0; y < h; ++y) {
        const QRgb *srcLine = reinterpret_cast<const QRgb*>(argb.constScanLine(y));
        QRgb *dstLine = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < w; ++x) {
            QRgb px = srcLine[x];
            dstLine[x] = qRgba(lut[qRed(px)], lut[qGreen(px)], lut[qBlue(px)], qAlpha(px));
        }
    }
    return dst;
}

// ====================================================================
// Auto Enhance — full pipeline
// ====================================================================
QImage ImageAlgorithm::autoEnhance(const QImage &src, double strength)
{
    if (src.isNull()) return {};
    double s = qBound(0.0, strength, 1.0);
    if (s < 0.005) return src;

    QImage result = src.convertToFormat(QImage::Format_ARGB32);

    // 1. Auto white balance (correct color casts)
    result = autoWhiteBalance(result);

    // 2. Auto levels (stretch histogram to fill full dynamic range)
    result = autoLevels(result, 0.1);

    // 3. S-curve for contrast enhancement
    if (s > 0.05) {
        double curveStr = 0.12 * s;
        QVector<QPointF> sCurve = {
            {0.0,   0.0},
            {0.25,  0.25 - curveStr * 0.25},
            {0.5,   0.5},
            {0.75,  0.75 + curveStr * 0.25},
            {1.0,   1.0}
        };
        result = toneCurve(result, sCurve, true);
    }

    // 4. Clarity (mid-frequency detail enhancement)
    if (s > 0.1) {
        double clarityAmt = 0.15 * s;
        result = clarity(result, clarityAmt, 5);
    }

    // 5. Saturation boost
    if (s > 0.05) {
        double satFactor = 1.0 + 0.2 * s;
        result = saturation(result, satFactor);
    }

    // 6. Blend with original by strength
    if (s < 1.0) {
        QImage orig = src.convertToFormat(QImage::Format_ARGB32);
        int w = result.width(), h = result.height();
        QImage blended(w, h, QImage::Format_ARGB32);
        for (int y = 0; y < h; ++y) {
            const QRgb *rLine = reinterpret_cast<const QRgb*>(result.constScanLine(y));
            const QRgb *oLine = reinterpret_cast<const QRgb*>(orig.constScanLine(y));
            QRgb *dLine = reinterpret_cast<QRgb*>(blended.scanLine(y));
            for (int x = 0; x < w; ++x) {
                QRgb rp = rLine[x], op = oLine[x];
                dLine[x] = qRgba(
                    clamp((int)(qRed(op)   * (1.0 - s) + qRed(rp)   * s)),
                    clamp((int)(qGreen(op) * (1.0 - s) + qGreen(rp) * s)),
                    clamp((int)(qBlue(op)  * (1.0 - s) + qBlue(rp)  * s)),
                    qAlpha(op));
            }
        }
        return blended;
    }

    return result;
}

// ====================================================================
// Oil Painting
// ====================================================================
QImage ImageAlgorithm::oilPaint(const QImage &src, int brushSize, int colorLevels)
{
    if (src.isNull()) return {};
    brushSize = qBound(1, brushSize, 20);
    colorLevels = qBound(2, colorLevels, 64);

    QImage input = src.convertToFormat(QImage::Format_ARGB32);
    int ow = input.width(), oh = input.height();
    int w = ow, h = oh;

    // Auto-downscale large images for performance
    QImage *workImg = &input;
    QImage scaled;
    const int maxPixels = 500000; // ~700x700
    if (ow * oh > maxPixels) {
        double scale = qSqrt((double)maxPixels / (ow * oh));
        w = qMax(64, (int)(ow * scale));
        h = qMax(64, (int)(oh * scale));
        scaled = input.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        workImg = &scaled;
        brushSize = qMax(1, (int)(brushSize * scale));
    }

    QImage result(w, h, QImage::Format_ARGB32);
    for (int y = 0; y < h; ++y) {
        QRgb *dLine = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < w; ++x) {
            QVector<int> count(colorLevels, 0);
            QVector<long long> sumR(colorLevels, 0);
            QVector<long long> sumG(colorLevels, 0);
            QVector<long long> sumB(colorLevels, 0);

            int y0 = qMax(0, y - brushSize), y1 = qMin(h - 1, y + brushSize);
            int x0 = qMax(0, x - brushSize), x1 = qMin(w - 1, x + brushSize);

            for (int sy = y0; sy <= y1; ++sy) {
                const QRgb *sLine = reinterpret_cast<const QRgb*>(workImg->constScanLine(sy));
                for (int sx = x0; sx <= x1; ++sx) {
                    QRgb p = sLine[sx];
                    int r = qRed(p), g = qGreen(p), b = qBlue(p);
                    int lum = (r * 299 + g * 587 + b * 114) / 1000;
                    int idx = qBound(0, lum * colorLevels / 256, colorLevels - 1);
                    count[idx]++;
                    sumR[idx] += r;
                    sumG[idx] += g;
                    sumB[idx] += b;
                }
            }

            int bestIdx = 0;
            for (int i = 1; i < colorLevels; ++i)
                if (count[i] > count[bestIdx]) bestIdx = i;

            if (count[bestIdx] > 0)
                dLine[x] = qRgba((int)(sumR[bestIdx] / count[bestIdx]),
                                 (int)(sumG[bestIdx] / count[bestIdx]),
                                 (int)(sumB[bestIdx] / count[bestIdx]), 255);
        }
    }

    // Scale back to original size if downscaled
    if (w != ow || h != oh)
        result = result.scaled(ow, oh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    return result;
}

// ====================================================================
// Polar Coordinates
// ====================================================================
QImage ImageAlgorithm::polarCoords(const QImage &src, bool polarToRect)
{
    if (src.isNull()) return {};
    QImage input = src.convertToFormat(QImage::Format_ARGB32);
    int w = input.width(), h = input.height();
    QImage result(w, h, QImage::Format_ARGB32);
    result.fill(0);

    double cx = w / 2.0, cy = h / 2.0;
    double maxR = qSqrt(cx*cx + cy*cy);

    for (int y = 0; y < h; ++y) {
        QRgb *dLine = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < w; ++x) {
            double sx, sy;
            bool outOfBounds = false;

            if (polarToRect) {
                double theta = (x / (double)w) * 2.0 * M_PI;
                double r = y / (double)h;
                sx = cx + r * qCos(theta) * maxR;
                sy = cy + r * qSin(theta) * maxR;
            } else {
                double dx = x - cx, dy = y - cy;
                double r = qSqrt(dx*dx + dy*dy) / maxR;
                if (r > 1.0) continue; // leave black
                double theta = qAtan2(dy, dx) / (2.0 * M_PI);
                if (theta < 0) theta += 1.0;
                sx = theta * w;
                sy = r * h;
            }

            // Bounds check
            if (sx < 0 || sx >= w || sy < 0 || sy >= h) { outOfBounds = true; }

            int ix, iy, ix2, iy2;
            if (polarToRect) {
                // X wraps around (angle), Y clamps
                double tw = w;
                ix = ((int)sx) % w; if (ix < 0) ix += w;
                iy = qBound(0, (int)sy, h - 1);
                ix2 = (ix + 1) % w;
                iy2 = qMin(iy + 1, h - 1);
            } else {
                ix = qBound(0, (int)sx, w - 1);
                iy = qBound(0, (int)sy, h - 1);
                ix2 = qMin(ix + 1, w - 1);
                iy2 = qMin(iy + 1, h - 1);
            }

            if (outOfBounds && !polarToRect) continue;

            double fx = sx - qFloor(sx), fy = sy - qFloor(sy);

            const QRgb *l0 = reinterpret_cast<const QRgb*>(input.constScanLine(iy));
            const QRgb *l1 = reinterpret_cast<const QRgb*>(input.constScanLine(iy2));

            double rr = (qRed(l0[ix])*(1-fx) + qRed(l0[ix2])*fx)*(1-fy)
                      + (qRed(l1[ix])*(1-fx) + qRed(l1[ix2])*fx)*fy;
            double gg = (qGreen(l0[ix])*(1-fx) + qGreen(l0[ix2])*fx)*(1-fy)
                      + (qGreen(l1[ix])*(1-fx) + qGreen(l1[ix2])*fx)*fy;
            double bb = (qBlue(l0[ix])*(1-fx) + qBlue(l0[ix2])*fx)*(1-fy)
                      + (qBlue(l1[ix])*(1-fx) + qBlue(l1[ix2])*fx)*fy;

            dLine[x] = qRgba(clamp((int)rr), clamp((int)gg), clamp((int)bb), 255);
        }
    }
    return result;
}

// ====================================================================
// Lens Flare
// ====================================================================
QImage ImageAlgorithm::lensFlare(const QImage &src, double cx, double cy,
                                  double brightness, double size)
{
    if (src.isNull()) return {};
    QImage input = src.convertToFormat(QImage::Format_ARGB32);
    int w = input.width(), h = input.height();
    QImage flare(w, h, QImage::Format_ARGB32);
    flare.fill(qRgba(0, 0, 0, 0));

    int px = qBound(0, (int)(cx * w), w - 1);
    int py = qBound(0, (int)(cy * h), h - 1);
    double s = qBound(0.1, size, 3.0) * qMax(w, h) / 500.0;
    double b = qBound(0.0, brightness, 2.0);

    QPainter p(&flare);
    p.setRenderHint(QPainter::Antialiasing);

    // 1. Central bright spot
    QRadialGradient centerGlow(px, py, 60 * s, px, py);
    centerGlow.setColorAt(0.0, QColor(255, 255, 255, (int)(200 * b)));
    centerGlow.setColorAt(0.1, QColor(255, 240, 200, (int)(150 * b)));
    centerGlow.setColorAt(0.5, QColor(255, 200, 150, (int)(60 * b)));
    centerGlow.setColorAt(1.0, QColor(255, 200, 150, 0));
    p.setBrush(centerGlow);
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPointF(px, py), 60 * s, 60 * s);

    // 2. Halo rings
    for (int i = 0; i < 3; ++i) {
        double r = (80 + i * 50) * s;
        QRadialGradient halo(px, py, r, px, py);
        int alpha = (int)((60 - i * 15) * b);
        halo.setColorAt(0.0, QColor(255, 255, 255, 0));
        halo.setColorAt(0.8, QColor(150, 180, 255, (int)(alpha * 0.5)));
        halo.setColorAt(1.0, QColor(255, 255, 255, 0));
        p.setBrush(halo);
        p.drawEllipse(QPointF(px, py), r, r);
    }

    // 3. Anamorphic streak
    QLinearGradient streakL(px - 200 * s, py, px + 200 * s, py);
    streakL.setColorAt(0.0, QColor(255, 100, 150, 0));
    streakL.setColorAt(0.3, QColor(200, 100, 255, (int)(40 * b)));
    streakL.setColorAt(0.5, QColor(255, 255, 255, (int)(80 * b)));
    streakL.setColorAt(0.7, QColor(100, 150, 255, (int)(40 * b)));
    streakL.setColorAt(1.0, QColor(255, 100, 150, 0));
    p.setBrush(streakL);
    p.drawRect(px - 200 * s, py - 2 * s, 400 * s, 4 * s);

    // 4. Chromatic artifacts opposite to center
    double oppositeX = w - px, oppositeY = h - py;
    for (int i = 0; i < 5; ++i) {
        double t = (double)(i + 1) / 6.0;
        double ax = px + (oppositeX - px) * t;
        double ay = py + (oppositeY - py) * t + (i - 2) * 15 * s;
        double ar = (8 - i) * s;
        QColor colors[] = { QColor(255, 100, 100), QColor(100, 200, 100),
                           QColor(100, 100, 255), QColor(255, 200, 100),
                           QColor(200, 100, 255) };
        QColor ac = colors[i % 5];
        ac.setAlpha((int)((60 - i * 10) * b));

        QRadialGradient ag(ax, ay, ar, ax, ay);
        ag.setColorAt(0.0, QColor(255, 255, 255, (int)((80 - i * 12) * b)));
        ag.setColorAt(0.5, ac);
        ag.setColorAt(1.0, QColor(ac.red(), ac.green(), ac.blue(), 0));
        p.setBrush(ag);
        p.drawEllipse(QPointF(ax, ay), ar, ar);
    }

    p.end();

    // Screen blend with original
    QImage result(w, h, QImage::Format_ARGB32);
    for (int y = 0; y < h; ++y) {
        const QRgb *sLine = reinterpret_cast<const QRgb*>(input.constScanLine(y));
        const QRgb *fLine = reinterpret_cast<const QRgb*>(flare.constScanLine(y));
        QRgb *dLine = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < w; ++x) {
            QRgb sp = sLine[x], fp = fLine[x];
            int fa = qAlpha(fp);
            if (fa == 0) { dLine[x] = sp; continue; }
            double blend = fa / 255.0;
            int r = clamp((int)(255 - (255 - qRed(sp)) * (1 - blend * qRed(fp)/255.0)));
            int g = clamp((int)(255 - (255 - qGreen(sp)) * (1 - blend * qGreen(fp)/255.0)));
            int b = clamp((int)(255 - (255 - qBlue(sp)) * (1 - blend * qBlue(fp)/255.0)));
            dLine[x] = qRgba(r, g, b, qAlpha(sp));
        }
    }
    return result;
}

// ====================================================================
// Metal Emboss — metallic color mapping + relief emboss
// ====================================================================
QImage ImageAlgorithm::metalEmboss(const QImage &src, int metalType,
                                    bool concave, double depth, double blend,
                                    double texture, double gloss)
{
    if (src.isNull()) return {};
    depth = qBound(1.0, depth, 20.0);
    blend = qBound(0.0, blend, 1.0);
    texture = qBound(0.0, texture, 1.0);
    gloss = qBound(0.0, gloss, 1.0);

    QImage input = src.convertToFormat(QImage::Format_ARGB32);
    int w = input.width(), h = input.height();

    // Convert to grayscale
    QImage gray(w, h, QImage::Format_Grayscale8);
    for (int y = 0; y < h; ++y) {
        const QRgb *sLine = reinterpret_cast<const QRgb*>(input.constScanLine(y));
        uchar *gLine = gray.scanLine(y);
        for (int x = 0; x < w; ++x) {
            QRgb p = sLine[x];
            gLine[x] = (uchar)((qRed(p)*299 + qGreen(p)*587 + qBlue(p)*114) / 1000);
        }
    }

    // Directional emboss kernel (top-left lighting):
    //   -1 -1  0
    //   -1  0  1
    //    0  1  1
    QImage emboss(w, h, QImage::Format_Grayscale8);
    int sign = concave ? -1 : 1;
    for (int y = 1; y < h - 1; ++y) {
        uchar *eLine = emboss.scanLine(y);
        for (int x = 1; x < w - 1; ++x) {
            int v = 0;
            v += -1 * gray.scanLine(y-1)[x-1];
            v += -1 * gray.scanLine(y-1)[x];
            v +=  0 * gray.scanLine(y-1)[x+1];
            v += -1 * gray.scanLine(y)[x-1];
            v +=  0 * gray.scanLine(y)[x];
            v +=  1 * gray.scanLine(y)[x+1];
            v +=  0 * gray.scanLine(y+1)[x-1];
            v +=  1 * gray.scanLine(y+1)[x];
            v +=  1 * gray.scanLine(y+1)[x+1];
            int val = (int)(v * sign * depth / 6.0) + 128;
            eLine[x] = (uchar)qBound(0, val, 255);
        }
    }
    // Fill border with 128 (neutral)
    for (int x = 0; x < w; ++x) {
        emboss.scanLine(0)[x] = 128;
        emboss.scanLine(h-1)[x] = 128;
    }
    for (int y = 0; y < h; ++y) {
        emboss.scanLine(y)[0] = 128;
        emboss.scanLine(y)[w-1] = 128;
    }

    // Apply S-curve contrast stretch to emboss for metallic specular response
    for (int y = 0; y < h; ++y) {
        uchar *eLine = emboss.scanLine(y);
        for (int x = 0; x < w; ++x) {
            double t = eLine[x] / 255.0;
            // Smoothstep S-curve: deepens shadows, sharpens highlights
            t = t * t * (3.0 - 2.0 * t);
            // Boost highlights above 75% for specular sharpness
            if (t > 0.75) t = t + (t - 0.75) * 1.5;
            eLine[x] = (uchar)qBound(0, (int)(t * 255.0), 255);
        }
    }

    // Metal color gradient stops — designed for metallic specular response
    QVector<QPair<double, QColor>> stops;
    switch (metalType) {
    case 1: // Silver — cold, highly reflective
        stops = { {0.0, QColor(18,18,22)}, {0.2, QColor(65,65,70)},
                  {0.4, QColor(130,130,135)}, {0.6, QColor(195,195,200)},
                  {0.78, QColor(235,235,238)}, {1.0, QColor(255,255,255)} };
        break;
    case 2: // 青铜色 — blue-green patina
        stops = { {0.0, QColor(10,40,30)}, {0.15, QColor(22,68,55)},
                  {0.3, QColor(38,100,78)}, {0.45, QColor(55,140,110)},
                  {0.6, QColor(85,180,148)}, {0.75, QColor(135,215,185)},
                  {0.88, QColor(175,237,215)}, {1.0, QColor(205,248,235)} };
        break;
    case 3: // 古铜色 — aged bronze
        stops = { {0.0, QColor(35,16,6)}, {0.15, QColor(72,35,14)},
                  {0.3, QColor(115,62,24)}, {0.45, QColor(160,100,38)},
                  {0.6, QColor(200,145,58)}, {0.75, QColor(235,188,95)},
                  {0.88, QColor(248,225,155)}, {1.0, QColor(255,245,210)} };
        break;
    default: // Gold — rich yellow-gold with sharp specular knee
        stops = { {0.0, QColor(30,18,0)}, {0.18, QColor(80,58,10)},
                  {0.38, QColor(155,122,30)}, {0.58, QColor(218,188,55)},
                  {0.76, QColor(252,235,140)}, {1.0, QColor(255,252,235)} };
        break;
    }

    // Apply gradient map (emboss grayscale → metal color)
    QImage metal = gradientMap(emboss, stops);

    // Patina texture for bronze types: blend with a color-shifted variant
    // using the original image's high-frequency detail as a mask.
    // This creates natural mottled copper-rust texture.
    if (texture > 0.01 && (metalType == 2 || metalType == 3)) {
        QVector<QPair<double, QColor>> texStops;
        if (metalType == 2) { // 青铜 — more blue/teal variation
            texStops = { {0.0, QColor(8,48,36)}, {0.15, QColor(15,85,68)},
                         {0.3, QColor(28,125,98)}, {0.45, QColor(40,168,135)},
                         {0.6, QColor(65,210,175)}, {0.75, QColor(105,238,210)},
                         {0.88, QColor(145,248,230)}, {1.0, QColor(185,252,242)} };
        } else { // 古铜 — more reddish/oxidized variation
            texStops = { {0.0, QColor(45,10,4)}, {0.15, QColor(88,22,10)},
                         {0.3, QColor(135,48,18)}, {0.45, QColor(180,80,28)},
                         {0.6, QColor(222,128,42)}, {0.75, QColor(245,178,78)},
                         {0.88, QColor(252,218,130)}, {1.0, QColor(255,242,200)} };
        }
        QImage metalVar = gradientMap(emboss, texStops);

        // Compute local detail map: per-pixel diff from 3×3 neighborhood average
        QImage detail(w, h, QImage::Format_Grayscale8);
        detail.fill(0);
        for (int y = 1; y < h - 1; ++y) {
            const uchar *g0 = gray.scanLine(y - 1);
            const uchar *g1 = gray.scanLine(y);
            const uchar *g2 = gray.scanLine(y + 1);
            uchar *dLine = detail.scanLine(y);
            for (int x = 1; x < w - 1; ++x) {
                int avg = (g0[x-1]+g0[x]+g0[x+1]+g1[x-1]+g1[x]+g1[x+1]+g2[x-1]+g2[x]+g2[x+1]) / 9;
                int diff = abs(g1[x] - avg);
                dLine[x] = (uchar)qMin(diff * 3, 255);
            }
        }

        // Blend base × variant using detail mask modulated by texture strength
        QImage textured(w, h, QImage::Format_ARGB32);
        for (int y = 0; y < h; ++y) {
            const QRgb *bLine = reinterpret_cast<const QRgb*>(metal.constScanLine(y));
            const QRgb *vLine = reinterpret_cast<const QRgb*>(metalVar.constScanLine(y));
            const uchar *dLine = detail.constScanLine(y);
            QRgb *tLine = reinterpret_cast<QRgb*>(textured.scanLine(y));
            for (int x = 0; x < w; ++x) {
                double t = (dLine[x] / 255.0) * texture;
                QRgb bp = bLine[x], vp = vLine[x];
                tLine[x] = qRgba(
                    clamp((int)(qRed(bp)*(1-t) + qRed(vp)*t)),
                    clamp((int)(qGreen(bp)*(1-t) + qGreen(vp)*t)),
                    clamp((int)(qBlue(bp)*(1-t) + qBlue(vp)*t)),
                    255);
            }
        }
        metal = textured;
    }

    // Gloss overlay — spherical highlight for metallic sheen (金/银)
    if (gloss > 0.01 && (metalType == 0 || metalType == 1)) {
        double cx = w / 2.0, cy = h / 2.0;
        double maxDist = qSqrt(cx*cx + cy*cy);
        // Gold gets warm highlight, silver gets cool white
        double hR = (metalType == 0) ? 1.0 : 1.0;
        double hG = (metalType == 0) ? 0.97 : 1.0;
        double hB = (metalType == 0) ? 0.84 : 1.0;

        for (int y = 0; y < h; ++y) {
            QRgb *mLine = reinterpret_cast<QRgb*>(metal.scanLine(y));
            const uchar *eLine = emboss.constScanLine(y);
            for (int x = 0; x < w; ++x) {
                double dx = (x - cx) / maxDist;
                double dy = (y - cy) / maxDist;
                double dist = qSqrt(dx*dx + dy*dy);
                double centerGlow = qExp(-dist * dist * 3.0) * 0.55;
                double rimGlow = qExp(-((dx+0.35)*(dx+0.35)*2.5 + (dy+0.35)*(dy+0.35)*2.5)) * 0.45;
                double g = (centerGlow + rimGlow) * gloss;
                g *= (0.25 + 0.75 * eLine[x] / 255.0); // raised areas glossier
                if (g > 0.005) {
                    QRgb mp = mLine[x];
                    int r = clamp((int)(255 - (255 - qRed(mp))   * (1.0 - g * hR)));
                    int gg = clamp((int)(255 - (255 - qGreen(mp)) * (1.0 - g * hG)));
                    int b = clamp((int)(255 - (255 - qBlue(mp))  * (1.0 - g * hB)));
                    mLine[x] = qRgba(r, gg, b, 255);
                }
            }
        }
    }

    // Blend with original
    if (blend >= 1.0) return metal;
    QImage result(w, h, QImage::Format_ARGB32);
    for (int y = 0; y < h; ++y) {
        const QRgb *mLine = reinterpret_cast<const QRgb*>(metal.constScanLine(y));
        const QRgb *sLine = reinterpret_cast<const QRgb*>(input.constScanLine(y));
        QRgb *dLine = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < w; ++x) {
            QRgb mp = mLine[x], sp = sLine[x];
            double b = blend;
            dLine[x] = qRgba(
                clamp((int)(qRed(mp)*b   + qRed(sp)*(1-b))),
                clamp((int)(qGreen(mp)*b + qGreen(sp)*(1-b))),
                clamp((int)(qBlue(mp)*b  + qBlue(sp)*(1-b))),
                qAlpha(sp));
        }
    }
    return result;
}

