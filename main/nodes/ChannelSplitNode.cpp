#include "ChannelSplitNode.h"
#include "algorithms/ImageAlgorithm.h"

ChannelSplitNode::ChannelSplitNode()
    : Node("通道分离")
{
    m_inputPorts = { Port("彩色图像", DataType::ColorImage, PortDirection::Input, 0) };
    m_outputPorts = {
        Port("R", DataType::GrayImage, PortDirection::Output, 0),
        Port("G", DataType::GrayImage, PortDirection::Output, 1),
        Port("B", DataType::GrayImage, PortDirection::Output, 2)
    };
}

bool ChannelSplitNode::process(const QVector<DataPacket> &inputs,
                               QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    QImage r, g, b;
    ImageAlgorithm::splitChannels(inputs[0].image(), r, g, b);
    if (r.isNull()) { errorMsg = "通道分离失败。"; return false; }
    outputs[0] = DataPacket(r);
    outputs[1] = DataPacket(g);
    outputs[2] = DataPacket(b);
    return true;
}
