#include "LensFlareNode.h"
#include "algorithms/ImageAlgorithm.h"

LensFlareNode::LensFlareNode()
    : Node("镜头光晕")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["posX"] = 0.5;
    m_params["posY"] = 0.3;
    m_params["brightness"] = 0.8;
    m_params["size"] = 1.0;
    setParamBound("posX", 0.0, 1.0, 0.01);
    setParamBound("posY", 0.0, 1.0, 0.01);
    setParamBound("brightness", 0.0, 2.0, 0.05);
    setParamBound("size", 0.1, 3.0, 0.05);
}

bool LensFlareNode::process(const QVector<DataPacket> &inputs,
                             QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    QImage result = ImageAlgorithm::lensFlare(inputs[0].image(),
        m_params["posX"].toDouble(), m_params["posY"].toDouble(),
        m_params["brightness"].toDouble(), m_params["size"].toDouble());
    if (result.isNull()) { errorMsg = "镜头光晕处理失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
