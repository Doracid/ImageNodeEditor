#pragma once

#include "Port.h"
#include "engine/DataPacket.h"
#include <QObject>
#include <QUuid>
#include <QMap>
#include <QVariant>
#include <QWidget>
#include <QVector>

class Node : public QObject {
    Q_OBJECT
public:
    explicit Node(const QString &title);
    ~Node() override = default;

    // Identity
    QString title() const { return m_title; }
    void setTitle(const QString &t) { m_title = t; }
    QUuid id() const { return m_id; }
    void setId(const QUuid &id) { m_id = id; }

    // Metadata (override in subclasses)
    virtual QString category() const = 0;   // "Input", "Output", "Filter", "Conversion", "MultiPort"
    virtual QString description() const = 0;

    // Ports
    const QVector<Port> &inputPorts() const { return m_inputPorts; }
    const QVector<Port> &outputPorts() const { return m_outputPorts; }

    // Parameters — stored in a map; subclasses may expose custom widgets
    void setParam(const QString &key, const QVariant &value);
    QVariant param(const QString &key, const QVariant &defaultVal = {}) const;
    QMap<QString, QVariant> allParams() const { return m_params; }

    // Optional custom parameter widget (return nullptr for auto-generated)
    virtual QWidget *createParamWidget() { return nullptr; }

    // The core processing method
    // Returns true on success, false on error (set errorMsg)
    virtual bool process(const QVector<DataPacket> &inputs,
                         QVector<DataPacket> &outputs,
                         QString &errorMsg) = 0;

    // Validation before execution
    virtual bool validate(QString &errorMsg);

    // Clone for workflow replay
    virtual Node *clone() const = 0;

signals:
    void paramsChanged();
    void outputUpdated(int portIndex);

protected:
    QString              m_title;
    QUuid                m_id;
    QVector<Port>        m_inputPorts;
    QVector<Port>        m_outputPorts;
    QMap<QString, QVariant> m_params;
};
