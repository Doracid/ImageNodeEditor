#include "ComicStyleNode.h"
#include "algorithms/ImageAlgorithm.h"

ComicStyleNode::ComicStyleNode()
    : Node("漫画风")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["edgeThreshold"] = 30;
    m_params["lineThickness"] = 2;
    setParamBound("edgeThreshold", 8, 255);
    setParamBound("lineThickness", 1, 5);
}

bool ComicStyleNode::process(const QVector<DataPacket> &inputs,
                             QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    int thr = m_params["edgeThreshold"].toInt();
    int thick = m_params["lineThickness"].toInt();
    QImage result = ImageAlgorithm::comicStyle(inputs[0].image(), thr, thick);
    if (result.isNull()) { errorMsg = "漫画风格处理失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
