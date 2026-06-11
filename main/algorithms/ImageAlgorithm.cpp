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
    // kernel: 0 -k  0
    //        -k 1+4k -k
    //         0 -k  0
    for (int y = 0; y < tmp.height(); ++y) {
        for (int x = 0; x < tmp.width(); ++x) {
            QRgb c = reinterpret_cast<QRgb*>(tmp.scanLine(y))[x];
            double r = qRed(c)   * (1.0 + 4.0 * k);
            double g = qGreen(c) * (1.0 + 4.0 * k);
            double b = qBlue(c)  * (1.0 + 4.0 * k);
            // Subtract neighbors
            if (x > 0) {
                QRgb p = reinterpret_cast<QRgb*>(tmp.scanLine(y))[x - 1];
                r -= k * qRed(p);   g -= k * qGreen(p);   b -= k * qBlue(p);
            }
            if (x < tmp.width() - 1) {
                QRgb p = reinterpret_cast<QRgb*>(tmp.scanLine(y))[x + 1];
                r -= k * qRed(p);   g -= k * qGreen(p);   b -= k * qBlue(p);
            }
            if (y > 0) {
                QRgb p = reinterpret_cast<QRgb*>(tmp.scanLine(y - 1))[x];
                r -= k * qRed(p);   g -= k * qGreen(p);   b -= k * qBlue(p);
            }
            if (y < tmp.height() - 1) {
                QRgb p = reinterpret_cast<QRgb*>(tmp.scanLine(y + 1))[x];
                r -= k * qRed(p);   g -= k * qGreen(p);   b -= k * qBlue(p);
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
// ====================================================================
QImage ImageAlgorithm::pencilSketch(const QImage &src, int blurRadius)
{
    if (src.isNull()) return {};
    int rad = qBound(1, blurRadius, 15);

    QImage gray = toGrayscale(src);
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
    for (int y = 0; y < gray.height(); ++y) {
        QRgb *gLine = reinterpret_cast<QRgb*>(gray.scanLine(y));
        QRgb *bLine = reinterpret_cast<QRgb*>(blurred.scanLine(y));
        QRgb *dLine = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < gray.width(); ++x) {
            int gv = qRed(gLine[x]);
            int bv = qRed(bLine[x]);
            int sketch = gv * 255 / qMax(bv + 1, 1);
            sketch = qMin(sketch, 255);
            dLine[x] = qRgba(sketch, sketch, sketch, 255);
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
