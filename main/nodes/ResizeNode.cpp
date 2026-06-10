#include "ResizeNode.h"
#include "algorithms/ImageAlgorithm.h"

ResizeNode::ResizeNode()
    : Node("缩放")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["width"]     = 512;
    m_params["height"]    = 512;
    m_params["keepRatio"] = false;
}

bool ResizeNode::process(const QVector<DataPacket> &inputs,
                         QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    QImage src = inputs[0].image();
    int w = m_params["width"].toInt();
    int h = m_params["height"].toInt();

    if (w <= 0 || h <= 0) { errorMsg = "无效的尺寸。"; return false; }

    auto mode = m_params["keepRatio"].toBool() ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio;
    QImage result = ImageAlgorithm::resize(src, w, h, mode);
    if (result.isNull()) { errorMsg = "Resize failed."; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
