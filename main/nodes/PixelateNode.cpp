#include "PixelateNode.h"
#include "algorithms/ImageAlgorithm.h"

PixelateNode::PixelateNode()
    : Node("像素化")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["blockSize"] = 8;
}

bool PixelateNode::process(const QVector<DataPacket> &inputs,
                           QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    int bs = m_params["blockSize"].toInt();
    QImage result = ImageAlgorithm::pixelate(inputs[0].image(), bs);
    if (result.isNull()) { errorMsg = "像素化失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
