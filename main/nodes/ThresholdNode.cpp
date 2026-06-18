#include "ThresholdNode.h"
#include "algorithms/ImageAlgorithm.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QVBoxLayout>

ThresholdNode::ThresholdNode()
    : Node("二值化")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["mode"] = 0;        // 0=global, 1=OTSU, 2=adaptive mean, 3=adaptive gaussian
    m_params["level"] = 128;     // only used in global mode
    m_params["blockSize"] = 15;  // block size for adaptive modes
    m_params["c"] = 8.0;         // C constant for adaptive modes
    setParamBound("level", 0, 255);
    setParamBound("blockSize", 3, 99);
    setParamBound("c", 0.0, 50.0, 0.5);
}

bool ThresholdNode::process(const QVector<DataPacket> &inputs,
                            QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    int mode = m_params["mode"].toInt();
    int level = m_params["level"].toInt();
    int blockSize = m_params["blockSize"].toInt();
    double c = m_params["c"].toDouble();
    QImage result = ImageAlgorithm::binarize(inputs[0].image(), mode, level, blockSize, c);
    if (result.isNull()) { errorMsg = "二值化处理失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}

QWidget *ThresholdNode::createParamWidget()
{
    auto *w = new QWidget();
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);

    // Mode combo
    auto *modeCombo = new QComboBox();
    modeCombo->addItem("全局阈值", 0);
    modeCombo->addItem("大津法 (OTSU)", 1);
    modeCombo->addItem("自适应均值", 2);
    modeCombo->addItem("自适应高斯", 3);
    modeCombo->setCurrentIndex(m_params["mode"].toInt());
    auto *modeLay = new QHBoxLayout();
    modeLay->addWidget(new QLabel("二值化类型:"));
    modeLay->addWidget(modeCombo, 1);
    lay->addLayout(modeLay);

    // Level spin (global only)
    auto *levelLay = new QHBoxLayout();
    levelLay->addWidget(new QLabel("阈值:"));
    auto *levelSpin = new QSpinBox();
    levelSpin->setRange(0, 255);
    levelSpin->setValue(m_params["level"].toInt());
    levelLay->addWidget(levelSpin);
    lay->addLayout(levelLay);

    // Block size (adaptive only)
    auto *blockLay = new QHBoxLayout();
    blockLay->addWidget(new QLabel("邻域大小:"));
    auto *blockSpin = new QSpinBox();
    blockSpin->setRange(3, 99);
    blockSpin->setSingleStep(2);
    blockSpin->setValue(m_params["blockSize"].toInt());
    blockLay->addWidget(blockSpin);
    lay->addLayout(blockLay);

    // C constant (adaptive only)
    auto *cLay = new QHBoxLayout();
    cLay->addWidget(new QLabel("C 常数:"));
    auto *cSpin = new QDoubleSpinBox();
    cSpin->setRange(0.0, 50.0);
    cSpin->setSingleStep(0.5);
    cSpin->setValue(m_params["c"].toDouble());
    cLay->addWidget(cSpin);
    lay->addLayout(cLay);

    // Show/hide params based on mode
    auto updateVisibility = [=](int modeIdx) {
        bool global = (modeIdx == 0);
        bool otsu = (modeIdx == 1);
        bool adaptive = (modeIdx == 2 || modeIdx == 3);
        levelLay->layout()->itemAt(1)->widget()->setVisible(global);
        levelLay->layout()->itemAt(0)->widget()->setVisible(global);
        blockLay->layout()->itemAt(1)->widget()->setVisible(adaptive);
        blockLay->layout()->itemAt(0)->widget()->setVisible(adaptive);
        cLay->layout()->itemAt(1)->widget()->setVisible(adaptive);
        cLay->layout()->itemAt(0)->widget()->setVisible(adaptive);
    };
    updateVisibility(m_params["mode"].toInt());

    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this, modeCombo, updateVisibility](int idx) {
        m_params["mode"] = modeCombo->itemData(idx).toInt();
        updateVisibility(idx);
        emit paramsChanged();
    });

    connect(levelSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int v) { m_params["level"] = v; emit paramsChanged(); });

    connect(blockSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int v) { m_params["blockSize"] = v; emit paramsChanged(); });

    connect(cSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            [this](double v) { m_params["c"] = v; emit paramsChanged(); });

    return w;
}
