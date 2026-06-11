#include "EdgeDetectionNode.h"
#include "algorithms/ImageAlgorithm.h"

EdgeDetectionNode::EdgeDetectionNode()
    : Node("边缘检测")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("边缘", DataType::GrayImage, PortDirection::Output, 0) };
    m_params["lowThreshold"]  = 50;
    m_params["highThreshold"] = 150;
    setParamBound("lowThreshold", 0, 255);
    setParamBound("highThreshold", 0, 255);
}

bool EdgeDetectionNode::process(const QVector<DataPacket> &inputs,
                                QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    int low  = m_params["lowThreshold"].toInt();
    int high = m_params["highThreshold"].toInt();
    if (low < 0 || high < 0 || low > 255 || high > 255) {
        errorMsg = "阈值必须在 0~255 之间。";
        return false;
    }
    QImage result = ImageAlgorithm::edgeDetection(inputs[0].image(), low, high);
    if (result.isNull()) { errorMsg = "边缘检测失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
