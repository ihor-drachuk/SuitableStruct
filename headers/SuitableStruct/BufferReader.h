/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <cassert>
#include <optional>
#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <SuitableStruct/Buffer.h>

namespace SuitableStruct {

class BufferReader
{
public:
    // Make sure lifetime of 'buffer' is greater than 'BufferReader's.
    BufferReader(const Buffer& buffer,
                 std::optional<size_t> optOffsetStart = {},
                 std::optional<size_t> optLen = {},
                 std::optional<size_t> optOffsetEnd = {})
        : m_buffer(buffer)
    {
        assert((!optLen && !optOffsetEnd) ||
               (optLen.has_value() ^ optOffsetEnd.has_value()));

        m_offsetStart = optOffsetStart.value_or(0);
        assert(m_offsetStart <= buffer.size());

        if (optLen) {
            m_optOffsetEnd = m_offsetStart + *optLen;
            assert(m_optOffsetEnd.value() <= buffer.size());
        } else if (optOffsetEnd) {
            m_optOffsetEnd = optOffsetEnd;
            assert(m_optOffsetEnd.value() <= buffer.size());
        }

        if (m_optOffsetEnd)
            assert(m_offsetStart <= m_optOffsetEnd.value());

        assert(position() <= size());
    }

    const Buffer& bufferSrc() const { return m_buffer; }
    Buffer bufferMapped() const { return Buffer(cdataSrc(), size()); }
    Buffer bufferRest() const { return Buffer(cdata(), rest()); }

    size_t offsetStart() const { return m_offsetStart; }
    size_t offsetEnd() const { return m_optOffsetEnd.value_or(m_buffer.size()); }

    size_t size() const { return offsetEnd() - offsetStart(); }
    size_t position() const { return m_position; }
    size_t rest() const { return size() - position(); }

    size_t seek(size_t pos) { checkPosition(pos); m_position = pos; return m_position; }
    size_t advance(std::ptrdiff_t delta) { checkAdvance(delta); return seek(m_position + delta); }
    void resetPosition() { m_position = 0; }

    const uint8_t* dataSrc() const { return m_buffer.data() + m_offsetStart; }
    const uint8_t* cdataSrc() const { return dataSrc(); }

    const uint8_t* data() const { return m_buffer.data() + m_offsetStart + m_position; }
    const uint8_t* cdata() const { return data(); }

    uint32_t hash() const;

    void readRaw(void* buffer, size_t sz) {
        checkAdvance(sz);
        memcpy(buffer, cdata(), sz);
        advance(sz);
    }

    BufferReader readRaw(size_t sz) {
        checkAdvance(sz);
        const BufferReader result(m_buffer, m_offsetStart + m_position, sz);
        advance(sz);
        return result;
    }

    template<typename T,
             typename std::enable_if_t<std::is_fundamental_v<T> || std::is_enum_v<T>>* = nullptr>
    void read(T& data) { readRaw(&data, sizeof(data)); }

    template<typename T,
             typename std::enable_if_t<std::is_fundamental_v<T> || std::is_enum_v<T>>* = nullptr>
    T read() {
        T data;
        read(data);
        return data;
    }

private:
    void checkPosition(size_t pos) const;
    void checkAdvance(std::ptrdiff_t delta) const;

private:
    const Buffer& m_buffer;
    size_t m_position { 0 };
    size_t m_offsetStart;
    std::optional<size_t> m_optOffsetEnd;
};

} // namespace SuitableStruct
