/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <cstdint>
#include <type_traits>
#include <SuitableStruct/Internals/LongSSO.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
class QByteArray;
#endif

static_assert (sizeof(char) == sizeof(uint8_t), "Buffer constructor (const char*, size) is wrong!");

namespace SuitableStruct {

class Buffer
{
public:
    Buffer() = default;
    Buffer(size_t size) { m_sso.allocate_copy(size); }
    Buffer(const uint8_t* data, size_t size) { m_sso.allocate_copy(size, data); }
    Buffer(const char* data, size_t size): Buffer(reinterpret_cast<const uint8_t*>(data), size) {}
    Buffer(const Buffer& rhs) = default;
    Buffer(Buffer&& rhs) = default;
    ~Buffer() = default;

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    Buffer(const QByteArray& buffer);
#endif

    [[nodiscard]] static Buffer fromConstChar(const char* str) {
        return Buffer(str, strlen(str));
    }

    template<typename T,
             typename std::enable_if_t<std::is_fundamental_v<T> || std::is_enum_v<T>>* = nullptr>
    [[nodiscard]] static Buffer fromValue(const T& value) {
        Buffer buffer;
        buffer.write(value);
        return buffer;
    }

    Buffer& operator= (const Buffer& rhs) = default;
    Buffer& operator= (Buffer&& rhs) = default;
    bool operator== (const Buffer& rhs) const;
    bool operator!= (const Buffer& rhs) const;
    Buffer& operator+= (const Buffer& rhs);
    Buffer& operator+= (Buffer&& rhs);

    friend Buffer operator+ (const Buffer& lhs, const Buffer& rhs) {
        Buffer result(lhs);
        result += rhs;
        return result;
    }

    friend Buffer operator+ (Buffer&& lhs, const Buffer& rhs) {
        lhs += rhs;
        return std::move(lhs);
    }

    friend Buffer operator+ (const Buffer& lhs, Buffer&& rhs) {
        Buffer result(lhs);
        result += std::move(rhs);
        return result;
    }

    friend Buffer operator+ (Buffer&& lhs, Buffer&& rhs) {
        lhs += std::move(rhs);
        return std::move(lhs);
    }

    void write(const Buffer& buffer);
    void write(Buffer&& buffer);
    template<typename T,
             typename std::enable_if_t<std::is_fundamental_v<T> || std::is_enum_v<T>>* = nullptr>
    void write(const T& data) { writeRaw(&data, sizeof(data)); }
    void writeZeros(size_t sz) { auto ptr = m_sso.allocate_copy(sz); memset(ptr, 0, sz); }

    void writeRaw(const void* ptr, size_t sz) { m_sso.allocate_copy(sz, static_cast<const uint8_t*>(ptr)); }

    uint32_t hash() const;
    uint32_t hashLegacy() const;
    size_t size() const { return m_sso.size(); }
    void reduceSize(size_t amount) { m_sso.reduceSize(amount); }

    uint8_t* allocate(size_t sz) { return m_sso.allocate_copy(sz); }
    const uint8_t* data() const { return m_sso.data(); }
    const uint8_t* cdata() const { return data(); }
    uint8_t* data() { return m_sso.data(); }

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    QByteArray toQByteArray() const;
    [[nodiscard]] static Buffer fromQByteArray(const QByteArray& buffer) { return Buffer(buffer); }
#endif

private:
    LongSSO<> m_sso;
};

} // namespace SuitableStruct
