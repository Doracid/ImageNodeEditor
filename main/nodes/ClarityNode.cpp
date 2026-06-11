#include "ClarityNode.h"
#include "algorithms/ImageAlgorithm.h"

ClarityNode::ClarityNode()
    : Node("清晰度")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["amount"] = 0.5;
    m_params["radius"] = 5;
    setParamBound("amount", 0.0, 1.0, 0.1);
    setParamBound("radius", 1, 30);
}

bool ClarityNode::process(const QVector<DataPacket> &inputs,
                          QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    double amount = m_params["amount"].toDouble();
    int radius = m_params["radius"].toInt();
    QImage result = ImageAlgorithm::clarity(inputs[0].image(), amount, radius);
    if (result.isNull()) { errorMsg = "清晰度调整失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
