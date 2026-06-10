#include "GrayscaleNode.h"
#include "algorithms/ImageAlgorithm.h"

GrayscaleNode::GrayscaleNode()
    : Node("灰度化")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("灰度图像", DataType::GrayImage, PortDirection::Output, 0) };
}

bool GrayscaleNode::process(const QVector<DataPacket> &inputs,
                            QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    QImage result = ImageAlgorithm::toGrayscale(inputs[0].image());
    if (result.isNull()) { errorMsg = "灰度转换失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
