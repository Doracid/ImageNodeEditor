#include "VibranceNode.h"
#include "algorithms/ImageAlgorithm.h"

VibranceNode::VibranceNode()
    : Node("自然饱和度")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["vibrance"] = 0.5;
    setParamBound("vibrance", 0.0, 1.0, 0.1);
}

bool VibranceNode::process(const QVector<DataPacket> &inputs,
                           QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    double amount = m_params["vibrance"].toDouble();
    QImage result = ImageAlgorithm::vibrance(inputs[0].image(), amount);
    if (result.isNull()) { errorMsg = "自然饱和度调整失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
