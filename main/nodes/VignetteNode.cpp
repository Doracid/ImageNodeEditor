#include "VignetteNode.h"
#include "algorithms/ImageAlgorithm.h"

VignetteNode::VignetteNode()
    : Node("暗角")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["radius"]   = 0.5;
    m_params["strength"] = 1.0;
    setParamBound("radius", 0.0, 1.0, 0.1);
    setParamBound("strength", 0.0, 5.0, 0.1);
}

bool VignetteNode::process(const QVector<DataPacket> &inputs,
                           QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    double radius   = m_params["radius"].toDouble();
    double strength = m_params["strength"].toDouble();
    QImage result = ImageAlgorithm::vignette(inputs[0].image(), radius, strength);
    if (result.isNull()) { errorMsg = "暗角处理失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
