#include "FadeNode.h"
#include "algorithms/ImageAlgorithm.h"

FadeNode::FadeNode()
    : Node("褪色")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["fade"] = 0.5;
}

bool FadeNode::process(const QVector<DataPacket> &inputs,
                       QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    double fade = m_params["fade"].toDouble();
    QImage result = ImageAlgorithm::fade(inputs[0].image(), fade);
    if (result.isNull()) { errorMsg = "褪色处理失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
