#include "AutoEnhanceNode.h"
#include "algorithms/ImageAlgorithm.h"

AutoEnhanceNode::AutoEnhanceNode()
    : Node("自动美化")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["strength"] = 0.5;
    setParamBound("strength", 0.0, 1.0, 0.1);
}

bool AutoEnhanceNode::process(const QVector<DataPacket> &inputs,
                              QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    double strength = m_params["strength"].toDouble();
    QImage result = ImageAlgorithm::autoEnhance(inputs[0].image(), strength);
    if (result.isNull()) { errorMsg = "自动美化处理失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
