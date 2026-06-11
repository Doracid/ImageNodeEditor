#include "WatermarkNode.h"
#include "algorithms/ImageAlgorithm.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QPushButton>
#include <QColorDialog>

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
    m_params["opacity"]  = 128;
    m_params["rotation"] = 0.0;
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
        0,   // posX — centered by default
        0,   // posY — centered by default
        m_params["rotation"].toDouble());
    if (result.isNull()) { errorMsg = "水印添加失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}

QWidget *WatermarkNode::createParamWidget()
{
    auto *w = new QWidget();
    auto *form = new QFormLayout(w);
    form->setContentsMargins(0, 0, 0, 0);

    // Text
    m_textEdit = new QLineEdit(m_params["text"].toString());
    connect(m_textEdit, &QLineEdit::textChanged, this, [this](const QString &t) {
        m_params["text"] = t;
        emit paramsChanged();
    });
    form->addRow("文字", m_textEdit);

    // Font size
    m_fontSizeSpin = new QSpinBox();
    m_fontSizeSpin->setRange(1, 999);
    m_fontSizeSpin->setValue(m_params["fontSize"].toInt());
    connect(m_fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int v) { m_params["fontSize"] = v; emit paramsChanged(); });
    form->addRow("字号", m_fontSizeSpin);

    // Rotation
    m_rotationSpin = new QDoubleSpinBox();
    m_rotationSpin->setRange(-360, 360);
    m_rotationSpin->setValue(m_params["rotation"].toDouble());
    m_rotationSpin->setSingleStep(1.0);
    m_rotationSpin->setSuffix("°");
    connect(m_rotationSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double v) { m_params["rotation"] = v; emit paramsChanged(); });
    form->addRow("旋转", m_rotationSpin);

    // Color preview button
    auto *colorBtn = new QPushButton("选择颜色");
    int r = m_params["red"].toInt(), g = m_params["green"].toInt(), b = m_params["blue"].toInt();
    colorBtn->setStyleSheet(QString("background-color: rgb(%1,%2,%3);").arg(r).arg(g).arg(b));
    connect(colorBtn, &QPushButton::clicked, this, [this, colorBtn]() {
        QColor cur(m_params["red"].toInt(), m_params["green"].toInt(), m_params["blue"].toInt());
        QColor c = QColorDialog::getColor(cur);
        if (c.isValid()) {
            m_params["red"]   = c.red();
            m_params["green"] = c.green();
            m_params["blue"]  = c.blue();
            colorBtn->setStyleSheet(QString("background-color: rgb(%1,%2,%3);").arg(c.red()).arg(c.green()).arg(c.blue()));
            emit paramsChanged();
        }
    });
    form->addRow("颜色", colorBtn);

    return w;
}
