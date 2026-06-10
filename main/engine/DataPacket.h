#pragma once

#include <QString>
#include <QImage>
#include <QVector>

// DataPacket carries the output of one node to the input of another.
// It holds either a single QImage or a list of QImages.
class DataPacket {
public:
    enum Type { Image, ImageList };

    DataPacket() : m_type(Image) {}
    explicit DataPacket(const QImage &img) : m_type(Image), m_image(img) {}
    explicit DataPacket(const QVector<QImage> &list) : m_type(ImageList), m_imageList(list) {}

    Type type() const { return m_type; }
    bool isValid() const {
        return m_type == Image ? !m_image.isNull() : !m_imageList.isEmpty();
    }

    QImage image() const { return m_image; }
    QVector<QImage> imageList() const { return m_imageList; }

    // Convenience: get first/last image regardless of type
    QImage firstImage() const {
        if (m_type == Image) return m_image;
        return m_imageList.isEmpty() ? QImage() : m_imageList.first();
    }

private:
    Type m_type = Image;
    QImage m_image;
    QVector<QImage> m_imageList;
};
