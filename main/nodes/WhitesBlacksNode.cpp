#include "WhitesBlacksNode.h"
#include "algorithms/ImageAlgorithm.h"

WhitesBlacksNode::WhitesBlacksNode()
    : Node("白色/黑色")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["whites"] = 0.0;
    m_params["blacks"] = 0.0;
    setParamBound("whites", 0.0, 1.0, 0.1);
    setParamBound("blacks", 0.0, 1.0, 0.1);
}

bool WhitesBlacksNode::process(const QVector<DataPacket> &inputs,
                               QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    double whites = m_params["whites"].toDouble();
    double blacks = m_params["blacks"].toDouble();
    QImage result = ImageAlgorithm::whitesBlacks(inputs[0].image(), whites, blacks);
    if (result.isNull()) { errorMsg = "白色/黑色调整失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
