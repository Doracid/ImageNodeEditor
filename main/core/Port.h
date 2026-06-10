#pragma once

#include "DataType.h"
#include <QString>

enum class PortDirection {
    Input,
    Output
};

struct Port {
    QString       name;
    DataType      dataType;
    PortDirection direction;
    int           index; // position within the node's port list

    Port() = default;
    Port(const QString &name, DataType type, PortDirection dir, int idx)
        : name(name), dataType(type), direction(dir), index(idx) {}
};
