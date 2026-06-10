#include "ImageOutputNode.h"
#include <QImage>
#include <QFileInfo>
#include <QDir>

ImageOutputNode::ImageOutputNode()
    : Node("图像输出")
{
    m_inputPorts = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_params["filePath"] = "";
}

static QString detectFormat(const QString &path)
{
    if (path.endsWith(".jpg", Qt::CaseInsensitive) || path.endsWith(".jpeg", Qt::CaseInsensitive))
        return "JPEG";
    if (path.endsWith(".bmp", Qt::CaseInsensitive)) return "BMP";
    if (path.endsWith(".tif", Qt::CaseInsensitive) || path.endsWith(".tiff", Qt::CaseInsensitive))
        return "TIFF";
    return "PNG";
}

bool ImageOutputNode::process(const QVector<DataPacket> &inputs,
                              QVector<DataPacket> &outputs,
                              QString &errorMsg)
{
    Q_UNUSED(outputs);
    if (!inputs[0].isValid()) {
        errorMsg = "没有输入图像可保存。";
        return false;
    }
    QString path = m_params.value("filePath").toString();
    if (path.isEmpty()) {
        errorMsg = "未指定输出文件路径。";
        return false;
    }

    // 确保目录存在
    QFileInfo fi(path);
    QDir().mkpath(fi.absolutePath());

    // 从扩展名自动判断格式，避免悬空指针
    QString format = detectFormat(path);
    if (!inputs[0].image().save(path, format.toLatin1().constData())) {
        errorMsg = QString("保存图像失败: %1").arg(path);
        return false;
    }
    return true;
}
