#include "InvertNode.h"
#include "algorithms/ImageAlgorithm.h"

InvertNode::InvertNode()
    : Node("反色")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
}

bool InvertNode::process(const QVector<DataPacket> &inputs,
                         QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    QImage result = ImageAlgorithm::invert(inputs[0].image());
    if (result.isNull()) { errorMsg = "反色处理失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
