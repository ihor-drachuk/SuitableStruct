#include <SuitableStruct/Internals/DefaultTypes.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QByteArray>
#include <QString>
#include <QPoint>
#include <QJsonObject>
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

namespace SuitableStruct {

Buffer ssSaveImpl(const std::string& value)
{
    Buffer buffer;
    buffer.write((uint64_t)value.size());
    buffer.writeRaw(value.data(), value.size());
    return buffer;
}

void ssLoadImpl(BufferReader& buffer, std::string& value)
{
    // Load & swap is not needed here because it's implemented in ssLoad

    uint64_t sz;
    buffer.read(sz);

    value.resize(sz);
    buffer.readRaw(value.data(), sz);
}


#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY

Buffer ssSaveImpl(const QByteArray& value)
{
    Buffer result;
    result.write((uint64_t)value.size());
    result.writeRaw(value.constData(), value.size());
    return result;
}

void ssLoadImpl(BufferReader& buffer, QByteArray& value)
{
    uint64_t sz;
    buffer.read(sz);
    value.resize(sz);
    buffer.readRaw(value.data(), sz);
}

Buffer ssSaveImpl(const QString& value)
{
    return ssSaveImpl(value.toUtf8());
}

void ssLoadImpl(BufferReader& buffer, QString& value)
{
    value = QString::fromUtf8(ssLoadImplRet<QByteArray>(buffer));
}

Buffer ssSaveImpl(const QPoint& value)
{
    Buffer result;
    result.write(value.x());
    result.write(value.y());
    return result;
}

void ssLoadImpl(BufferReader& buffer, QPoint& value)
{
    ssLoadImpl(buffer, value.rx());
    ssLoadImpl(buffer, value.ry());
}

QJsonValue ssJsonSaveImpl(QChar value)
{
    return QString(value);
}

void ssJsonLoadImpl(const QJsonValue& src, QChar& dst)
{
    assert(src.toString().length() == 1);
    dst = src.toString().at(0);
}

QJsonValue ssJsonSaveImpl(const QString& value)
{
    return value;
}

void ssJsonLoadImpl(const QJsonValue& src, QString& dst)
{
    dst = src.toString();
}

QJsonValue ssJsonSaveImpl(const QByteArray& value)
{
    const auto result = QString::fromLatin1(value.toHex());
    return result.isEmpty() ? result : ("0x" + result);
}

void ssJsonLoadImpl(const QJsonValue& src, QByteArray& dst)
{
    assert(src.isString());
    const auto str = src.toString();

    if (str.isEmpty()) {
        dst.clear();
        return;
    }

    assert(str.mid(0, 2) == "0x");
    dst = QByteArray::fromHex(str.mid(2).toLatin1());
}

QJsonValue ssJsonSaveImpl(const std::string& value)
{
    return QString::fromStdString(value);
}

void ssJsonLoadImpl(const QJsonValue& src, std::string& dst)
{
    assert(src.isString());
    dst = src.toString().toStdString();
}

QJsonValue ssJsonSaveImpl(const QPoint& value)
{
    QJsonObject obj;
    obj["x"] = ssJsonSaveImpl(value.x());
    obj["y"] = ssJsonSaveImpl(value.y());
    return obj;
}

void ssJsonLoadImpl(const QJsonValue& src, QPoint& dst)
{
    assert(src.isObject());
    const auto obj = src.toObject();
    ssJsonLoadImpl(obj["x"], dst.rx());
    ssJsonLoadImpl(obj["y"], dst.ry());
}

#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

} // namespace SuitableStruct
