#include "GaussianBlurNode.h"
#include "algorithms/ImageAlgorithm.h"

GaussianBlurNode::GaussianBlurNode()
    : Node("高斯模糊")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["radius"] = 5;
}

bool GaussianBlurNode::process(const QVector<DataPacket> &inputs,
                               QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    int radius = m_params["radius"].toInt();
    if (radius < 1) { errorMsg = "半径必须 >= 1。"; return false; }
    QImage result = ImageAlgorithm::gaussianBlur(inputs[0].image(), radius);
    if (result.isNull()) { errorMsg = "模糊处理失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
