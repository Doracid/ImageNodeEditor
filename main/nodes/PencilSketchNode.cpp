#include "PencilSketchNode.h"
#include "algorithms/ImageAlgorithm.h"

PencilSketchNode::PencilSketchNode()
    : Node("铅笔画")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["blurRadius"] = 3;
    m_params["detailBoost"] = 0.5;
    setParamBound("blurRadius", 1, 15);
    setParamBound("detailBoost", 0.0, 1.0, 0.1);
}

bool PencilSketchNode::process(const QVector<DataPacket> &inputs,
                               QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    int rad = m_params["blurRadius"].toInt();
    double boost = m_params["detailBoost"].toDouble();
    QImage result = ImageAlgorithm::pencilSketch(inputs[0].image(), rad, boost);
    if (result.isNull()) { errorMsg = "铅笔画处理失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
