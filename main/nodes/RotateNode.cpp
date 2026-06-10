#include "RotateNode.h"
#include "algorithms/ImageAlgorithm.h"

RotateNode::RotateNode()
    : Node("旋转")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["angle"] = 90.0;
    m_params["bgRed"]   = 255;
    m_params["bgGreen"] = 255;
    m_params["bgBlue"]  = 255;
}

bool RotateNode::process(const QVector<DataPacket> &inputs,
                         QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    double angle = m_params["angle"].toDouble();
    QColor bg(m_params["bgRed"].toInt(), m_params["bgGreen"].toInt(), m_params["bgBlue"].toInt());
    QImage result = ImageAlgorithm::rotate(inputs[0].image(), angle, bg);
    if (result.isNull()) { errorMsg = "旋转失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
