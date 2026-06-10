#pragma once

#include "core/Node.h"
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>

class WatermarkNode : public Node {
    Q_OBJECT
public:
    WatermarkNode();
    QString category() const override { return "几何变换"; }
    QString description() const override { return "Add text watermark to image."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new WatermarkNode(); }
    QWidget *createParamWidget() override;

private:
    QLineEdit *m_textEdit = nullptr;
    QSpinBox *m_fontSizeSpin = nullptr;
    QDoubleSpinBox *m_rotationSpin = nullptr;
};
