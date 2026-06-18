#include "OilPaintingNode.h"
#include "algorithms/ImageAlgorithm.h"

OilPaintingNode::OilPaintingNode()
    : Node("油画效果")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["brushSize"] = 5;
    m_params["colorLevels"] = 8;
    setParamBound("brushSize", 1, 20);
    setParamBound("colorLevels", 2, 64);
}

bool OilPaintingNode::process(const QVector<DataPacket> &inputs,
                               QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    QImage result = ImageAlgorithm::oilPaint(inputs[0].image(),
        m_params["brushSize"].toInt(), m_params["colorLevels"].toInt());
    if (result.isNull()) { errorMsg = "油画处理失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
