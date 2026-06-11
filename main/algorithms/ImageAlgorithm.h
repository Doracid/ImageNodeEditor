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
    // saturation: 0=gray, 1=original, >1=oversaturated
    static QImage saturation(const QImage &src, double factor);
    // hueShift: 0..360 degrees
    static QImage hueShift(const QImage &src, int angle);

    // ---- Tone / Mapping ----
    static QImage gammaCorrection(const QImage &src, double gamma);
    static QImage threshold(const QImage &src, int level);
    static QImage sepia(const QImage &src, double intensity);

    // ---- Filters ----
    static QImage gaussianBlur(const QImage &src, int radius);
    static QImage edgeDetection(const QImage &src, int lowThreshold, int highThreshold);
    static QImage sharpen(const QImage &src, double strength);

    // ---- Stylize ----
    // Temperature: -1=cool, 0=neutral, +1=warm
    static QImage colorTemperature(const QImage &src, double temperature);
    // Fade: 0=original, 1=fully desaturated to gray
    static QImage fade(const QImage &src, double fade);
    // Pixelate: blockSize >= 2
    static QImage pixelate(const QImage &src, int blockSize);
    // Vignette: radius[0,1], strength[0,5]
    static QImage vignette(const QImage &src, double radius, double strength);
    // Pencil sketch: blurRadius[1,15]
    static QImage pencilSketch(const QImage &src, int blurRadius);
    // Cartoon: edgeThreshold[0,255], levels[2,16]
    static QImage cartoon(const QImage &src, int edgeThreshold, int levels);

    // ---- Exposure ----
    // EV: -5..+5 (0 = no change)
    static QImage exposure(const QImage &src, double ev);

    // ---- Vibrance ----
    // amount: 0=no change, 1=max boost (protects skin tones & saturated areas)
    static QImage vibrance(const QImage &src, double amount);

    // ---- White Balance ----
    // temperature: -1(cool/blue) ~ 0(neutral) ~ +1(warm/yellow)
    // tint:       -1(green)      ~ 0(neutral) ~ +1(magenta)
    static QImage whiteBalance(const QImage &src, double temperature, double tint);
    // Auto white balance using gray-world assumption
    static QImage autoWhiteBalance(const QImage &src);

    // ---- Highlights / Shadows ----
    // highlights: -1..+1 (positive = darken highlights)
    // shadows:    -1..+1 (positive = brighten shadows)
    static QImage highlightsShadows(const QImage &src, double highlights, double shadows);

    // ---- Whites / Blacks (endpoint clipping) ----
    // whites: 0..1 (push white point)
    // blacks: 0..1 (push black point)
    static QImage whitesBlacks(const QImage &src, double whites, double blacks);

    // ---- Clarity (mid-frequency contrast) ----
    // amount: 0..1, radius: 1..20 (typical ~5)
    static QImage clarity(const QImage &src, double amount, int radius);

    // ---- Tone Curve (256-level LUT from control points) ----
    // points: list of (x,y) in [0,1]; mono=true → same curve for all channels
    static QImage toneCurve(const QImage &src, const QVector<QPointF> &points, bool mono = true);

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
                               int posX, int posY, double rotation = 0.0);

private:
    // Helper: 1D Gaussian kernel weights
    static QVector<double> gaussianKernel(int radius);
    // Helper: clamp value to byte range
    static int clamp(int v) { return qBound(0, v, 255); }
};
