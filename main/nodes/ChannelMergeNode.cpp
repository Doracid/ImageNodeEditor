#include "ChannelMergeNode.h"
#include "algorithms/ImageAlgorithm.h"

ChannelMergeNode::ChannelMergeNode()
    : Node("通道合并")
{
    m_inputPorts = {
        Port("R", DataType::GrayImage, PortDirection::Input, 0),
        Port("G", DataType::GrayImage, PortDirection::Input, 1),
        Port("B", DataType::GrayImage, PortDirection::Input, 2)
    };
    m_outputPorts = { Port("彩色图像", DataType::ColorImage, PortDirection::Output, 0) };
}

bool ChannelMergeNode::process(const QVector<DataPacket> &inputs,
                               QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid() || !inputs[1].isValid() || !inputs[2].isValid()) {
        errorMsg = "需要全部三个通道的输入。";
        return false;
    }
    QImage result = ImageAlgorithm::mergeChannels(
        inputs[0].image(), inputs[1].image(), inputs[2].image());
    if (result.isNull()) { errorMsg = "通道合并失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
