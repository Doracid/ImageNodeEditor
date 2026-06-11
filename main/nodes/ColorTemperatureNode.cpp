#include "ColorTemperatureNode.h"
#include "algorithms/ImageAlgorithm.h"

ColorTemperatureNode::ColorTemperatureNode()
    : Node("冷暖色调")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["temperature"] = 0.0;
    setParamBound("temperature", -1.0, 1.0, 0.1);
}

bool ColorTemperatureNode::process(const QVector<DataPacket> &inputs,
                                   QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    double t = m_params["temperature"].toDouble();
    QImage result = ImageAlgorithm::colorTemperature(inputs[0].image(), t);
    if (result.isNull()) { errorMsg = "色调调整失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
