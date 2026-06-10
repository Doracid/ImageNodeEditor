#include "HueShiftNode.h"
#include "algorithms/ImageAlgorithm.h"

HueShiftNode::HueShiftNode()
    : Node("色相偏移")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["angle"] = 0;
}

bool HueShiftNode::process(const QVector<DataPacket> &inputs,
                           QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    int angle = m_params["angle"].toInt();
    QImage result = ImageAlgorithm::hueShift(inputs[0].image(), angle);
    if (result.isNull()) { errorMsg = "色相偏移失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
