#include "ImageInputNode.h"
#include <QImage>
#include <QImageReader>
#include <QFileInfo>

ImageInputNode::ImageInputNode()
    : Node("图像输入")
{
    m_outputPorts = { Port("图像", DataType::ColorImage, PortDirection::Output, 0) };
    m_params["filePath"] = "";
}

bool ImageInputNode::process(const QVector<DataPacket> &inputs,
                             QVector<DataPacket> &outputs,
                             QString &errorMsg)
{
    Q_UNUSED(inputs);
    QString path = m_params.value("filePath").toString();
    if (path.isEmpty()) {
        errorMsg = "未指定文件路径。";
        return false;
    }

    QFileInfo fi(path);
    if (!fi.exists()) {
        errorMsg = QString("文件不存在: %1").arg(path);
        return false;
    }

    QImageReader reader(path);
    reader.setAutoTransform(true);
    QImage img = reader.read();
    if (img.isNull()) {
        // Check if format is supported
        QString fmt = reader.format();
        QStringList supported;
        for (const auto &b : QImageReader::supportedImageFormats())
            supported << QString::fromLatin1(b);

        errorMsg = QString("无法加载图像: %1\n文件格式: %2\nQt 支持的格式: %3\n错误: %4")
                       .arg(path)
                       .arg(fmt.isEmpty() ? fi.suffix() : fmt)
                       .arg(supported.join(", "))
                       .arg(reader.errorString());
        return false;
    }
    outputs[0] = DataPacket(img.convertToFormat(QImage::Format_ARGB32));
    return true;
}
