#include "ImageInputNode.h"
#include <QImage>
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
    QImage img(path);
    if (img.isNull()) {
        errorMsg = QString("无法加载图像: %1").arg(path);
        return false;
    }
    outputs[0] = DataPacket(img.convertToFormat(QImage::Format_ARGB32));
    return true;
}
