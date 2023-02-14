#pragma once
#include <cassert>
#include <optional>
#include <cstdint>
#include <type_traits>
#include <SuitableStruct/Buffer.h>

namespace SuitableStruct {

class BufferReader
{
public:
    // Make sure lifetime of 'buffer' is greater than 'BufferReader's.
    inline BufferReader(const Buffer& buffer,
                        std::optional<size_t> offsetStart = {},
                        std::optional<size_t> len = {},
                        std::optional<size_t> offsetEnd = {})
        : m_buffer(buffer)
    {
        assert((!len && !offsetEnd) ||
               (len.has_value() ^ offsetEnd.has_value()));

        m_offsetStart = offsetStart.value_or(0);

        if (len) {
            m_offsetEnd = m_offsetStart + *len;
        } else if (offsetEnd) {
            m_offsetEnd = offsetEnd;
        }

        if (m_offsetEnd)
            assert(m_offsetStart <= m_offsetEnd.value());

        assert(position() <= size());
    }

    inline const Buffer& bufferSrc() const { return m_buffer; }
    inline Buffer bufferMapped() const { return Buffer(cdataSrc(), size()); }
    inline Buffer bufferRest() const { return Buffer(cdata(), rest()); }

    inline size_t offsetStart() const { return m_offsetStart; };
    inline size_t offsetEnd() const { return m_offsetEnd.value_or(m_buffer.size()); }

    inline size_t size() const { return offsetEnd() - offsetStart(); };
    inline size_t position() const { return m_offset; }
    inline size_t rest() const { return size() - position(); };

    inline size_t seek(size_t pos) { assert(pos <= size()); m_offset = pos; return m_offset; };
    inline size_t advance(int64_t delta) { assert( static_cast<int64_t>(m_offset) + delta >= 0); return seek(m_offset + delta); }
    inline void resetPosition() { m_offset = 0; }

    inline const uint8_t* dataSrc() const { return m_buffer.data() + m_offsetStart; }
    inline const uint8_t* cdataSrc() const { return dataSrc(); }

    inline const uint8_t* data() const { return m_buffer.data() + m_offsetStart + m_offset; }
    inline const uint8_t* cdata() const { return data(); }

    uint32_t hash() const;

    void readRaw(void* buffer, size_t sz) {
        checkPosition(sz);
        memcpy(buffer, cdata(), sz);
        advance(sz);
    }

    BufferReader readRaw(size_t sz) {
        checkPosition(sz);
        const BufferReader result(m_buffer, m_offsetStart + m_offset, sz);
        advance(sz);
        return result;
    }

    template<typename T,
             typename std::enable_if_t<std::is_fundamental_v<T> || std::is_enum_v<T>>* = nullptr>
    void read(T& data) { readRaw(&data, sizeof(data)); }

private:
    void checkPosition(size_t sz) const;

private:
    const Buffer& m_buffer;
    size_t m_offset { 0 };
    size_t m_offsetStart;
    std::optional<size_t> m_offsetEnd;
};

} // namespace SuitableStruct
