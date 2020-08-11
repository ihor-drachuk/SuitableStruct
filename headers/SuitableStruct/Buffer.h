#pragma once
#include <cstdint>
#include <type_traits>
#include <SuitableStruct/LongSSO.h>

namespace SuitableStruct {

class Buffer
{
public:
    inline Buffer() = default;
    inline Buffer(size_t size) { m_sso.allocate_copy(size); }
    inline Buffer(const Buffer& rhs) = default;
    inline Buffer(Buffer&& rhs) = default;
    inline ~Buffer() = default;

    template<typename T,
             typename std::enable_if_t<std::is_fundamental<T>::value>* = nullptr>
    [[nodiscard]] static Buffer fromValue(const T& value) {
        Buffer buffer;
        buffer.write(value);
        return buffer;
    }

    Buffer& operator= (const Buffer& rhs) = default;
    Buffer& operator= (Buffer&& rhs) = default;
    bool operator== (const Buffer& rhs);
    bool operator!= (const Buffer& rhs);
    Buffer& operator+= (const Buffer& rhs);
    Buffer& operator+= (Buffer&& rhs);

    void write(const Buffer& buffer);
    void write(Buffer&& buffer);
    template<typename T,
             typename std::enable_if_t<std::is_fundamental<T>::value>* = nullptr>
    void write(const T& data) { writeRaw(&data, sizeof(data)); }

    inline void writeRaw(const void* ptr, size_t sz) { m_sso.allocate_copy(sz, static_cast<const uint8_t*>(ptr)); }

    uint32_t hash() const;
    inline size_t size() const { return m_sso.size(); }
    inline void reduceSize(size_t amount) { m_sso.reduceSize(amount); }

    inline uint8_t* allocate(size_t sz) { return m_sso.allocate_copy(sz); };
    inline const uint8_t* data() const { return m_sso.data(); };
    inline const uint8_t* cdata() const { return data(); }
    inline uint8_t* data() { return m_sso.data(); }

private:
    LongSSO<> m_sso;
};

} // namespace SuitableStruct
