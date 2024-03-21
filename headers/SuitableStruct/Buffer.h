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
    inline Buffer() = default;
    inline Buffer(size_t size) { m_sso.allocate_copy(size); }
    inline Buffer(const uint8_t* data, size_t size) { m_sso.allocate_copy(size, data); }
    inline Buffer(const char* data, size_t size): Buffer(reinterpret_cast<const uint8_t*>(data), size) {}
    inline Buffer(const Buffer& rhs) = default;
    inline Buffer(Buffer&& rhs) = default;
    inline ~Buffer() = default;

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

    void write(const Buffer& buffer);
    void write(Buffer&& buffer);
    template<typename T,
             typename std::enable_if_t<std::is_fundamental_v<T> || std::is_enum_v<T>>* = nullptr>
    void write(const T& data) { writeRaw(&data, sizeof(data)); }

    inline void writeRaw(const void* ptr, size_t sz) { m_sso.allocate_copy(sz, static_cast<const uint8_t*>(ptr)); }

    uint32_t hash() const;
    inline size_t size() const { return m_sso.size(); }
    inline void reduceSize(size_t amount) { m_sso.reduceSize(amount); }

    inline uint8_t* allocate(size_t sz) { return m_sso.allocate_copy(sz); };
    inline const uint8_t* data() const { return m_sso.data(); };
    inline const uint8_t* cdata() const { return data(); }
    inline uint8_t* data() { return m_sso.data(); }

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    QByteArray toQByteArray() const;
    [[nodiscard]] static Buffer fromQByteArray(const QByteArray& buffer) { return Buffer(buffer); }
#endif

private:
    LongSSO<> m_sso;
};

} // namespace SuitableStruct
