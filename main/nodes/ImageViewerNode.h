#pragma once

#include "core/Node.h"

class ImageViewerNode : public Node {
    Q_OBJECT
public:
    ImageViewerNode();
    QString category() const override { return "Output"; }
    QString description() const override { return "Preview image in a window."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs,
                 QString &errorMsg) override;
    Node *clone() const override { return new ImageViewerNode(); }

    QImage lastImage() const { return m_lastImage; }

signals:
    void imageReady(const QImage &img);

private:
    QImage m_lastImage;
};
