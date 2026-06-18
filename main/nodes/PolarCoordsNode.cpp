#include "PolarCoordsNode.h"
#include "algorithms/ImageAlgorithm.h"

#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>

PolarCoordsNode::PolarCoordsNode()
    : Node("极坐标")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["mode"] = 0; // 0=矩形→极坐标, 1=极坐标→矩形
}

bool PolarCoordsNode::process(const QVector<DataPacket> &inputs,
                               QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    bool polarToRect = (m_params["mode"].toInt() == 1);
    QImage result = ImageAlgorithm::polarCoords(inputs[0].image(), polarToRect);
    if (result.isNull()) { errorMsg = "极坐标处理失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}

QWidget *PolarCoordsNode::createParamWidget()
{
    auto *w = new QWidget();
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);

    auto *combo = new QComboBox();
    combo->addItem("矩形→极坐标", 0);
    combo->addItem("极坐标→矩形", 1);
    combo->setCurrentIndex(m_params["mode"].toInt());
    lay->addWidget(new QLabel("转换模式:"));
    lay->addWidget(combo);

    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this, combo](int idx) {
                m_params["mode"] = combo->itemData(idx).toInt();
                emit paramsChanged();
            });

    return w;
}
