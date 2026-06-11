#include "WhiteBalanceNode.h"
#include "algorithms/ImageAlgorithm.h"

WhiteBalanceNode::WhiteBalanceNode()
    : Node("白平衡")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["temperature"] = 0.0;
    m_params["tint"]       = 0.0;
    m_params["auto"]       = false;
    setParamBound("temperature", -1.0, 1.0, 0.1);
    setParamBound("tint", -1.0, 1.0, 0.1);
}

bool WhiteBalanceNode::process(const QVector<DataPacket> &inputs,
                               QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }

    QImage result;
    if (m_params["auto"].toBool()) {
        result = ImageAlgorithm::autoWhiteBalance(inputs[0].image());
    } else {
        double temperature = m_params["temperature"].toDouble();
        double tint = m_params["tint"].toDouble();
        result = ImageAlgorithm::whiteBalance(inputs[0].image(), temperature, tint);
    }

    if (result.isNull()) { errorMsg = "白平衡调整失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
