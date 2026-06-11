#include "HighlightsShadowsNode.h"
#include "algorithms/ImageAlgorithm.h"

HighlightsShadowsNode::HighlightsShadowsNode()
    : Node("高光/阴影")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["highlights"] = 0.0;
    m_params["shadows"]    = 0.0;
    setParamBound("highlights", -1.0, 1.0, 0.1);
    setParamBound("shadows", -1.0, 1.0, 0.1);
}

bool HighlightsShadowsNode::process(const QVector<DataPacket> &inputs,
                                    QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    double highlights = m_params["highlights"].toDouble();
    double shadows = m_params["shadows"].toDouble();
    QImage result = ImageAlgorithm::highlightsShadows(inputs[0].image(), highlights, shadows);
    if (result.isNull()) { errorMsg = "高光/阴影调整失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
