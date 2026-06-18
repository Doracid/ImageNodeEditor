#include "MetalStyleNode.h"
#include "algorithms/ImageAlgorithm.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QVBoxLayout>

MetalStyleNode::MetalStyleNode()
    : Node("金属底板")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["metalType"] = 1;    // 0=gold, 1=silver, 2=bronze, 3=ancient-bronze
    m_params["embossType"] = 0;   // 0=凸版, 1=凹版
    m_params["depth"] = 10;
    m_params["blend"] = 0.8;
    m_params["texture"] = 0.6;
    m_params["gloss"] = 0.5;
    setParamBound("depth", 1, 20);
    setParamBound("blend", 0.0, 1.0, 0.05);
    setParamBound("texture", 0.0, 1.0, 0.05);
    setParamBound("gloss", 0.0, 1.0, 0.05);
}

bool MetalStyleNode::process(const QVector<DataPacket> &inputs,
                              QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    QImage result = ImageAlgorithm::metalEmboss(inputs[0].image(),
        m_params["metalType"].toInt(),
        m_params["embossType"].toInt() == 1,
        m_params["depth"].toInt(),
        m_params["blend"].toDouble(),
        m_params["texture"].toDouble(),
        m_params["gloss"].toDouble());
    if (result.isNull()) { errorMsg = "金属风格处理失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}

QWidget *MetalStyleNode::createParamWidget()
{
    auto *w = new QWidget();
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);

    // Metal type
    auto *metalCombo = new QComboBox();
    metalCombo->addItem("金色", 0);
    metalCombo->addItem("银色", 1);
    metalCombo->addItem("青铜色", 2);
    metalCombo->addItem("古铜色", 3);
    metalCombo->setCurrentIndex(metalCombo->findData(m_params["metalType"].toInt()));
    auto *metalLay = new QHBoxLayout();
    metalLay->addWidget(new QLabel("金属类型:"));
    metalLay->addWidget(metalCombo, 1);
    lay->addLayout(metalLay);

    connect(metalCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this, metalCombo](int idx) {
                m_params["metalType"] = metalCombo->itemData(idx).toInt();
                emit paramsChanged();
            });

    // Emboss type
    auto *embossCombo = new QComboBox();
    embossCombo->addItem("凸版", 0);
    embossCombo->addItem("凹版", 1);
    embossCombo->setCurrentIndex(m_params["embossType"].toInt());
    auto *embossLay = new QHBoxLayout();
    embossLay->addWidget(new QLabel("浮雕类型:"));
    embossLay->addWidget(embossCombo, 1);
    lay->addLayout(embossLay);

    connect(embossCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this, embossCombo](int idx) {
                m_params["embossType"] = embossCombo->itemData(idx).toInt();
                emit paramsChanged();
            });

    // Depth
    auto *depthLay = new QHBoxLayout();
    depthLay->addWidget(new QLabel("浮雕深度:"));
    auto *depthSpin = new QSpinBox();
    depthSpin->setRange(1, 20);
    depthSpin->setValue(m_params["depth"].toInt());
    depthLay->addWidget(depthSpin);
    lay->addLayout(depthLay);

    connect(depthSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int v) { m_params["depth"] = v; emit paramsChanged(); });

    // Blend
    auto *blendLay = new QHBoxLayout();
    blendLay->addWidget(new QLabel("混合程度:"));
    auto *blendSpin = new QDoubleSpinBox();
    blendSpin->setRange(0.0, 1.0);
    blendSpin->setSingleStep(0.05);
    blendSpin->setValue(m_params["blend"].toDouble());
    blendLay->addWidget(blendSpin);
    lay->addLayout(blendLay);

    connect(blendSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            [this](double v) { m_params["blend"] = v; emit paramsChanged(); });

    // Texture (patina detail, only affects 青铜/古铜)
    auto *texLay = new QHBoxLayout();
    texLay->addWidget(new QLabel("铜锈纹理:"));
    auto *texSpin = new QDoubleSpinBox();
    texSpin->setRange(0.0, 1.0);
    texSpin->setSingleStep(0.05);
    texSpin->setValue(m_params["texture"].toDouble());
    texLay->addWidget(texSpin);
    lay->addLayout(texLay);

    connect(texSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            [this](double v) { m_params["texture"] = v; emit paramsChanged(); });

    // Gloss (spherical highlight, mainly affects 金/银)
    auto *glossLay = new QHBoxLayout();
    glossLay->addWidget(new QLabel("光泽强度:"));
    auto *glossSpin = new QDoubleSpinBox();
    glossSpin->setRange(0.0, 1.0);
    glossSpin->setSingleStep(0.05);
    glossSpin->setValue(m_params["gloss"].toDouble());
    glossLay->addWidget(glossSpin);
    lay->addLayout(glossLay);

    connect(glossSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            [this](double v) { m_params["gloss"] = v; emit paramsChanged(); });

    return w;
}
