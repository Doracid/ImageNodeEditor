#include "BrightnessContrastNode.h"
#include "algorithms/ImageAlgorithm.h"

BrightnessContrastNode::BrightnessContrastNode()
    : Node("亮度/对比度")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["brightness"] = 0;
    m_params["contrast"]   = 1.0;
    setParamBound("brightness", -255, 255);
    setParamBound("contrast", 0.0, 3.0, 0.1);
}

bool BrightnessContrastNode::process(const QVector<DataPacket> &inputs,
                                     QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    int brightness = m_params["brightness"].toInt();
    double contrast = m_params["contrast"].toDouble();
    QImage result = ImageAlgorithm::brightnessContrast(inputs[0].image(), brightness, contrast);
    if (result.isNull()) { errorMsg = "调整失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
