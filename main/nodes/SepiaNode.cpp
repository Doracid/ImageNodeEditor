#include "SepiaNode.h"
#include "algorithms/ImageAlgorithm.h"

SepiaNode::SepiaNode()
    : Node("怀旧")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["intensity"] = 1.0;
    setParamBound("intensity", 0.0, 1.0, 0.1);
}

bool SepiaNode::process(const QVector<DataPacket> &inputs,
                        QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    double intensity = m_params["intensity"].toDouble();
    QImage result = ImageAlgorithm::sepia(inputs[0].image(), intensity);
    if (result.isNull()) { errorMsg = "怀旧效果处理失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
