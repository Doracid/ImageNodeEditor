#pragma once

#include <QString>

enum class DataType {
    ColorImage,  // RGB color image (QImage Format_ARGB32)
    GrayImage,   // Grayscale image (QImage Format_Grayscale8)
    Any          // Wildcard — matches everything
};

inline QString dataTypeName(DataType t) {
    switch (t) {
        case DataType::ColorImage: return "ColorImage";
        case DataType::GrayImage:  return "GrayImage";
        case DataType::Any:        return "Any";
    }
    return "Unknown";
}

// Returns true if data of sourceType can be connected to a port of targetType
inline bool isTypeCompatible(DataType source, DataType target) {
    if (source == DataType::Any || target == DataType::Any) return true;
    return source == target;
}
