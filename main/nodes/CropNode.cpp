#include "CropNode.h"
#include "algorithms/ImageAlgorithm.h"

CropNode::CropNode()
    : Node("裁剪")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["x"] = 0;
    m_params["y"] = 0;
    m_params["width"]  = 256;
    m_params["height"] = 256;
}

bool CropNode::process(const QVector<DataPacket> &inputs,
                       QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    QImage src = inputs[0].image();
    int x = m_params["x"].toInt();
    int y = m_params["y"].toInt();
    int w = m_params["width"].toInt();
    int h = m_params["height"].toInt();

    if (x + w > src.width() || y + h > src.height()) {
        errorMsg = QString("Crop region (%1,%2 %3x%4) exceeds image bounds (%5x%6)")
                       .arg(x).arg(y).arg(w).arg(h).arg(src.width()).arg(src.height());
        return false;
    }

    QImage result = ImageAlgorithm::crop(src, x, y, w, h);
    if (result.isNull()) { errorMsg = "Crop failed."; return false; }
    outputs[0] = DataPacket(result);
    return true;
}
