#include "ThresholdNode.h"
#include "algorithms/ImageAlgorithm.h"

ThresholdNode::ThresholdNode()
    : Node("阈值")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["level"] = 128;
}

bool ThresholdNode::process(const QVector<DataPacket> &inputs,
                            QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    int level = m_params["level"].toInt();
    QImage result = ImageAlgorithm::threshold(inputs[0].image(), level);
    if (result.isNull()) { errorMsg = "阈值处理失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
