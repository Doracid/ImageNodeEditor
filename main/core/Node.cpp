#include "Node.h"
#include <QUuid>

Node::Node(const QString &title)
    : m_title(title)
    , m_id(QUuid::createUuid())
{
}

void Node::setParam(const QString &key, const QVariant &value)
{
    m_params[key] = value;
    emit paramsChanged();
}

QVariant Node::param(const QString &key, const QVariant &defaultVal) const
{
    return m_params.value(key, defaultVal);
}

bool Node::validate(QString &errorMsg)
{
    Q_UNUSED(errorMsg);
    return true;
}

void Node::setParamBound(const QString &key, const QVariant &min, const QVariant &max, double step)
{
    ParamBound b;
    b.min = min;
    b.max = max;
    b.step = step;
    m_paramBounds[key] = b;
}

ParamBound Node::paramBound(const QString &key) const
{
    return m_paramBounds.value(key, ParamBound());
}
