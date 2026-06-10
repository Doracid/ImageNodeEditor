#include "SaturationNode.h"
#include "algorithms/ImageAlgorithm.h"

SaturationNode::SaturationNode()
    : Node("饱和度")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["saturation"] = 1.0;
}

bool SaturationNode::process(const QVector<DataPacket> &inputs,
                             QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    double factor = m_params["saturation"].toDouble();
    QImage result = ImageAlgorithm::saturation(inputs[0].image(), factor);
    if (result.isNull()) { errorMsg = "饱和度调整失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
