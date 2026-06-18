#pragma once

#include "core/Node.h"

class GradientMapNode : public Node {
    Q_OBJECT
public:
    GradientMapNode();
    QString category() const override { return "色彩调整"; }
    QString description() const override { return "渐变映射：将灰度映射为彩色渐变。"; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new GradientMapNode(); }

    // Serialize/deserialize gradient stops to/from param string
    static QString stopsToString(const QVector<QPair<double, QColor>> &stops);
    static QVector<QPair<double, QColor>> stringToStops(const QString &str);

    QWidget *createParamWidget() override;
};
