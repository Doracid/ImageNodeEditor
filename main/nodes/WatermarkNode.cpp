#include "WatermarkNode.h"
#include "algorithms/ImageAlgorithm.h"

WatermarkNode::WatermarkNode()
    : Node("水印")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["text"]     = "水印";
    m_params["fontSize"] = 48;
    m_params["red"]      = 255;
    m_params["green"]    = 255;
    m_params["blue"]     = 255;
    m_params["opacity"]  = 128;     // 0..255
    m_params["posX"]     = 0;       // 0 = 图片中心
    m_params["posY"]     = 0;       // 0 = 图片中心
}

bool WatermarkNode::process(const QVector<DataPacket> &inputs,
                            QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    QColor color(m_params["red"].toInt(), m_params["green"].toInt(),
                 m_params["blue"].toInt());
    QImage result = ImageAlgorithm::addWatermark(
        inputs[0].image(),
        m_params["text"].toString(),
        m_params["fontSize"].toInt(),
        color,
        m_params["opacity"].toInt(),
        m_params["posX"].toInt(),
        m_params["posY"].toInt());
    if (result.isNull()) { errorMsg = "水印添加失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
