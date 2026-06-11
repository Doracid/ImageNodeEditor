#include "ResizeNode.h"
#include "algorithms/ImageAlgorithm.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFormLayout>

ResizeNode::ResizeNode()
    : Node("缩放")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["width"]     = 512;
    m_params["height"]    = 512;
    m_params["keepRatio"] = false;
}

bool ResizeNode::process(const QVector<DataPacket> &inputs,
                         QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    QImage src = inputs[0].image();

    // Update aspect ratio from original image for UI coupling
    m_aspectRatio = (double)src.width() / src.height();

    int w = m_params["width"].toInt();
    int h = m_params["height"].toInt();
    if (w <= 0 || h <= 0) { errorMsg = "无效的尺寸。"; return false; }

    auto mode = m_params["keepRatio"].toBool() ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio;
    QImage result = ImageAlgorithm::resize(src, w, h, mode);
    if (result.isNull()) { errorMsg = "缩放失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}

QWidget *ResizeNode::createParamWidget()
{
    auto *w = new QWidget();
    auto *form = new QFormLayout(w);
    form->setContentsMargins(0, 0, 0, 0);

    // Width
    m_wSpin = new QSpinBox();
    m_wSpin->setRange(1, 100000);
    m_wSpin->setValue(m_params["width"].toInt());
    connect(m_wSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int v) {
        m_params["width"] = v;
        if (m_keepCb && m_keepCb->isChecked()) updateHeightFromWidth();
        emit paramsChanged();
    });
    form->addRow("宽度", m_wSpin);

    // Height
    m_hSpin = new QSpinBox();
    m_hSpin->setRange(1, 100000);
    m_hSpin->setValue(m_params["height"].toInt());
    connect(m_hSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int v) {
        m_params["height"] = v;
        if (m_keepCb && m_keepCb->isChecked()) updateWidthFromHeight();
        emit paramsChanged();
    });
    form->addRow("高度", m_hSpin);

    // Keep ratio
    m_keepCb = new QCheckBox("保持宽高比");
    m_keepCb->setChecked(m_params["keepRatio"].toBool());
    connect(m_keepCb, &QCheckBox::toggled, this, [this](bool v) {
        m_params["keepRatio"] = v;
        emit paramsChanged();
    });
    form->addRow("", m_keepCb);

    return w;
}

void ResizeNode::updateHeightFromWidth()
{
    if (m_aspectRatio > 0.001 && m_hSpin) {
        int newH = qMax(1, (int)(m_wSpin->value() / m_aspectRatio));
        m_hSpin->blockSignals(true);
        m_hSpin->setValue(newH);
        m_hSpin->blockSignals(false);
        m_params["height"] = newH;
    }
}

void ResizeNode::updateWidthFromHeight()
{
    if (m_aspectRatio > 0.001 && m_wSpin) {
        int newW = qMax(1, (int)(m_hSpin->value() * m_aspectRatio));
        m_wSpin->blockSignals(true);
        m_wSpin->setValue(newW);
        m_wSpin->blockSignals(false);
        m_params["width"] = newW;
    }
}
