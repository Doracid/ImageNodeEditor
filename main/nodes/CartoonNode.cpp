#include "CartoonNode.h"
#include "algorithms/ImageAlgorithm.h"

CartoonNode::CartoonNode()
    : Node("卡通风格")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["edgeThreshold"] = 80;
    m_params["levels"]        = 6;
}

bool CartoonNode::process(const QVector<DataPacket> &inputs,
                          QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    int thr   = m_params["edgeThreshold"].toInt();
    int levels = m_params["levels"].toInt();
    QImage result = ImageAlgorithm::cartoon(inputs[0].image(), thr, levels);
    if (result.isNull()) { errorMsg = "卡通风格处理失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
