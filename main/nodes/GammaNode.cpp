#include "GammaNode.h"
#include "algorithms/ImageAlgorithm.h"

GammaNode::GammaNode()
    : Node("伽马校正")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["gamma"] = 1.0;
    setParamBound("gamma", 0.1, 5.0, 0.1);
}

bool GammaNode::process(const QVector<DataPacket> &inputs,
                        QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    double gamma = m_params["gamma"].toDouble();
    QImage result = ImageAlgorithm::gammaCorrection(inputs[0].image(), gamma);
    if (result.isNull()) { errorMsg = "伽马校正失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
