#include "ImageViewerNode.h"

ImageViewerNode::ImageViewerNode()
    : Node("图像查看器")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
}

bool ImageViewerNode::process(const QVector<DataPacket> &inputs,
                              QVector<DataPacket> &outputs,
                              QString &errorMsg)
{
    if (!inputs[0].isValid()) {
        errorMsg = "没有输入图像可显示。";
        return false;
    }
    m_lastImage = inputs[0].image();
    outputs[0] = inputs[0]; // pass through for downstream preview
    emit imageReady(m_lastImage);
    return true;
}
