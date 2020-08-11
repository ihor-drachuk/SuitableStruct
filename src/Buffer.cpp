#include <SuitableStruct/Buffer.h>
#include <SuitableStruct/Hashes.h>

namespace SuitableStruct {

bool Buffer::operator==(const Buffer& rhs)
{
    return m_sso == rhs.m_sso;
}

bool Buffer::operator!=(const Buffer& rhs)
{
    return m_sso != rhs.m_sso;
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
    const auto sz = size();
    return sz ? ssHashRaw(data(), sz) : 0;
}

} // namespace SuitableStruct
