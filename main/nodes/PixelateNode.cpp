#include "PixelateNode.h"
#include "algorithms/ImageAlgorithm.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

PixelateNode::PixelateNode()
    : Node("像素画")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["blockSize"] = 8;
    m_params["shapeMode"] = 0;  // 0=正方形, 1=圆角方形, 2=圆形
    m_params["showOutline"] = false; // true=有轮廓, false=无轮廓
    m_params["outlineWidth"] = 1;    // 轮廓粗细(px)
    m_params["bgColor"] = 0;     // 0=白色, 1=黑色
    setParamBound("blockSize", 2, 200);
    setParamBound("outlineWidth", 1, 20);
}

bool PixelateNode::process(const QVector<DataPacket> &inputs,
                           QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    int bs = m_params["blockSize"].toInt();
    int shapeMode = m_params["shapeMode"].toInt();
    bool showOutline = m_params["showOutline"].toBool();
    int outlineWidth = m_params["outlineWidth"].toInt();
    bool bgWhite = (m_params["bgColor"].toInt() == 0);
    QImage result = ImageAlgorithm::pixelArt(inputs[0].image(), bs, shapeMode, showOutline, outlineWidth, bgWhite);
    if (result.isNull()) { errorMsg = "像素画处理失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}

QWidget *PixelateNode::createParamWidget()
{
    auto *w = new QWidget();
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);

    // Block size
    auto *bsLay = new QHBoxLayout();
    bsLay->addWidget(new QLabel("像素块大小:"));
    auto *bsSpin = new QSpinBox();
    bsSpin->setRange(2, 200);
    bsSpin->setValue(m_params["blockSize"].toInt());
    bsLay->addWidget(bsSpin);
    lay->addLayout(bsLay);

    connect(bsSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int v) { m_params["blockSize"] = v; emit paramsChanged(); });

    // Shape mode
    auto *shapeCombo = new QComboBox();
    shapeCombo->addItem("正方形", 0);
    shapeCombo->addItem("圆角方形", 1);
    shapeCombo->addItem("圆形", 2);
    shapeCombo->setCurrentIndex(shapeCombo->findData(m_params["shapeMode"].toInt()));
    auto *shapeLay = new QHBoxLayout();
    shapeLay->addWidget(new QLabel("像素形状:"));
    shapeLay->addWidget(shapeCombo, 1);
    lay->addLayout(shapeLay);

    connect(shapeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this, shapeCombo](int idx) {
                m_params["shapeMode"] = shapeCombo->itemData(idx).toInt();
                emit paramsChanged();
            });

    // Outline toggle
    auto *outlineCb = new QCheckBox("有轮廓（黑线勾勒）");
    outlineCb->setChecked(m_params["showOutline"].toBool());
    auto *outlineLay = new QHBoxLayout();
    outlineLay->addWidget(new QLabel("轮廓:"));
    outlineLay->addWidget(outlineCb, 1);
    lay->addLayout(outlineLay);

    connect(outlineCb, &QCheckBox::toggled, this,
            [this](bool v) { m_params["showOutline"] = v; emit paramsChanged(); });

    // Outline width
    auto *owLay = new QHBoxLayout();
    owLay->addWidget(new QLabel("轮廓粗细:"));
    auto *owSpin = new QSpinBox();
    owSpin->setRange(1, 20);
    owSpin->setValue(m_params["outlineWidth"].toInt());
    owSpin->setSuffix(" px");
    owLay->addWidget(owSpin);
    lay->addLayout(owLay);

    connect(owSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int v) { m_params["outlineWidth"] = v; emit paramsChanged(); });

    // Background color
    auto *bgCombo = new QComboBox();
    bgCombo->addItem("白色", 0);
    bgCombo->addItem("黑色", 1);
    bgCombo->setCurrentIndex(bgCombo->findData(m_params["bgColor"].toInt()));
    auto *bgLay = new QHBoxLayout();
    bgLay->addWidget(new QLabel("背景颜色:"));
    bgLay->addWidget(bgCombo, 1);
    lay->addLayout(bgLay);

    connect(bgCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this, bgCombo](int idx) {
                m_params["bgColor"] = bgCombo->itemData(idx).toInt();
                emit paramsChanged();
            });

    return w;
}
