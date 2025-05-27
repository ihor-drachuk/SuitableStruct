/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <SuitableStruct/Buffer.h>
#include <SuitableStruct/Hashes.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QByteArray>
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

namespace SuitableStruct {

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
Buffer::Buffer(const QByteArray& buffer)
    : Buffer(reinterpret_cast<const uint8_t*>(buffer.constData()), buffer.size())
{
}
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

bool Buffer::operator==(const Buffer& rhs) const
{
    return m_sso == rhs.m_sso;
}

bool Buffer::operator!=(const Buffer& rhs) const
{
    return !(*this == rhs);
}

Buffer& Buffer::operator+= (const Buffer& rhs)
{
    if (this == &rhs) return *this;
    write(rhs);
    return *this;
}

void Buffer::write(const Buffer& buffer)
{
    writeRaw(buffer.cdata(), buffer.size());
}

void Buffer::write(Buffer&& buffer)
{
    if (size()) {
        writeRaw(buffer.cdata(), buffer.size());
    } else {
        m_sso = std::move(buffer.m_sso);
    }
}

Buffer& Buffer::operator+= (Buffer&& rhs)
{
    if (this == &rhs) return *this;
    write(std::move(rhs));
    return *this;
}

uint32_t Buffer::hash() const
{
    return ssHashRaw(data(), size());
}

uint32_t Buffer::hashLegacy() const
{
    return ssHashRaw_F0(data(), size());
}

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
QByteArray Buffer::toQByteArray() const
{
    return QByteArray(reinterpret_cast<const char*>(cdata()), size());
}
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

} // namespace SuitableStruct
