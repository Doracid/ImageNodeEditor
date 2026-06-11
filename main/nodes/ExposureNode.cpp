#include "ExposureNode.h"
#include "algorithms/ImageAlgorithm.h"

ExposureNode::ExposureNode()
    : Node("曝光")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["ev"] = 0.0;
    setParamBound("ev", -5.0, 5.0, 0.1);
}

bool ExposureNode::process(const QVector<DataPacket> &inputs,
                           QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    double ev = m_params["ev"].toDouble();
    QImage result = ImageAlgorithm::exposure(inputs[0].image(), ev);
    if (result.isNull()) { errorMsg = "曝光调整失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
