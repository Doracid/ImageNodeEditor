#include "RotateNode.h"
#include "algorithms/ImageAlgorithm.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFormLayout>

RotateNode::RotateNode()
    : Node("旋转")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["angle"]   = 90.0;
    m_params["bgColor"] = QStringLiteral("white");
}

bool RotateNode::process(const QVector<DataPacket> &inputs,
                         QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    double angle = m_params["angle"].toDouble();
    QString bgStr = m_params["bgColor"].toString();

    QColor bg;
    if (bgStr == "transparent") {
        bg = QColor(0, 0, 0, 0);
    } else if (bgStr == "black") {
        bg = Qt::black;
    } else {
        bg = Qt::white;
    }

    QImage result = ImageAlgorithm::rotate(inputs[0].image(), angle, bg);
    if (result.isNull()) { errorMsg = "旋转失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}

QWidget *RotateNode::createParamWidget()
{
    auto *w = new QWidget();
    auto *form = new QFormLayout(w);
    form->setContentsMargins(0, 0, 0, 0);

    // Angle
    m_angleSpin = new QDoubleSpinBox();
    m_angleSpin->setRange(-36000, 36000);
    m_angleSpin->setValue(m_params["angle"].toDouble());
    m_angleSpin->setSingleStep(1.0);
    m_angleSpin->setSuffix("°");
    connect(m_angleSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double v) { m_params["angle"] = v; emit paramsChanged(); });
    form->addRow("旋转角度", m_angleSpin);

    // Background color
    m_bgCombo = new QComboBox();
    m_bgCombo->addItem("白色", QStringLiteral("white"));
    m_bgCombo->addItem("黑色", QStringLiteral("black"));
    m_bgCombo->addItem("透明", QStringLiteral("transparent"));
    QString curBg = m_params["bgColor"].toString();
    m_bgCombo->setCurrentIndex(m_bgCombo->findData(curBg));
    connect(m_bgCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
        m_params["bgColor"] = m_bgCombo->itemData(idx).toString();
        emit paramsChanged();
    });
    form->addRow("背景色", m_bgCombo);

    return w;
}
