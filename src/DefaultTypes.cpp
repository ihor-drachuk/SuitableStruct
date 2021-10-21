#include <SuitableStruct/DefaultTypes.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QByteArray>
#include <QString>
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
    QByteArray temp;

    uint64_t sz;
    buffer.read(sz);
    temp.resize(sz);

    buffer.readRaw(temp.data(), sz);

    value = std::move(temp);
}

Buffer ssSaveImpl(const QString& value)
{
    return ssSaveImpl(value.toUtf8());
}

void ssLoadImpl(BufferReader& buffer, QString& value)
{
    value = QString::fromUtf8(ssLoadImplRet<QByteArray>(buffer));
}

#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

} // namespace SuitableStruct
