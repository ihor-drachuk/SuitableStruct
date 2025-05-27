/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <SuitableStruct/Handlers.h>
#include <SuitableStruct/Internals/Helpers.h>
#include <cstdint>
#include <type_traits>
#include <memory>
#include <optional>


namespace SuitableStruct {

// ssHash. Forward
template<typename T>
uint32_t ssHash(const T& value);

// ssHash. Tools
uint32_t ssHashRaw_F0(const void* ptr, size_t sz); // Legacy
uint32_t ssHashRaw_F1(const void* ptr, size_t sz);
uint32_t ssHashRaw(const void* ptr, size_t sz);

namespace Internal {

// Smart pointer hash implementations - hash the content, not the pointer
template<typename T>
uint32_t ssHashImpl(const std::shared_ptr<T>& value)
{
    return value ? ssHash(*value) : 0;
}

template<typename T>
uint32_t ssHashImpl(const std::unique_ptr<T>& value)
{
    return value ? ssHash(*value) : 0;
}

template<typename T>
uint32_t ssHashImpl(const std::optional<T>& value)
{
    return value ? ssHash(*value) : 0;
}

template<typename T,
         typename std::enable_if<
             !::SuitableStruct::Handlers<T>::value &&
             !can_ssHashImpl<T>::value &&
             !can_ssTuple<T>::value
             >::type* = nullptr>
uint32_t ssHashImpl(const T& obj)
{
    return std::hash<T>{}(obj);
}

inline uint32_t combineHash(uint32_t hash, uint32_t seed)
{
    return seed ^ (hash + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}


// ssHash. Tuple implementation
template<size_t I = 0, typename... Args,
         typename std::enable_if<I == sizeof...(Args)>::type* = nullptr>
uint32_t ssHashImpl(const std::tuple<Args...>&)
{
    return 0;
}

template<size_t I = 0, typename... Args,
         typename std::enable_if<!(I >= sizeof...(Args))>::type* = nullptr>
uint32_t ssHashImpl(const std::tuple<Args...>& args)
{
    return combineHash(ssHash(std::get<I>(args)), ssHashImpl<I+1>(args));
}

// ssHash.  1) Method
template<typename T,
         typename std::enable_if<can_ssHashImpl<T>::value>::type* = nullptr>
uint32_t ssHashImpl(const T& obj)
{
    return obj.ssHashImpl();
}

// ssHash.  2) Handlers
template<typename T,
         typename std::enable_if<
             ::SuitableStruct::Handlers<T>::value &&
             !can_ssHashImpl<T>::value
             >::type* = nullptr>
uint32_t ssHashImpl(const T& obj)
{
    return ::SuitableStruct::Handlers<T>::ssHashImpl(obj);
}

// ssHash.  3) Tuple
template<typename T,
         typename std::enable_if<
             !::SuitableStruct::Handlers<T>::value &&
             !can_ssHashImpl<T>::value &&
              can_ssTuple<T>::value
             >::type* = nullptr>
uint32_t ssHashImpl(const T& obj)
{
    return ssHashImpl(obj.ssTuple());
}

} // namespace Internal

template<typename T>
uint32_t ssHash(const T& value)
{
    return Internal::ssHashImpl(value);
}

} // namespace SuitableStruct


#define SS_HASHES(STRUCT) \
    inline unsigned int qHash(const STRUCT& s, unsigned int seed = 0) \
    { \
        return SuitableStruct::Internal::combineHash(SuitableStruct::Internal::ssHashImpl(s), seed); \
    } \
     \
    namespace std { \
    template<> \
    struct hash<STRUCT> \
    { \
        std::size_t operator()(const STRUCT& k) const \
        { \
            return SuitableStruct::Internal::ssHashImpl(k); \
        } \
    }; \
    } /*namespace std*/
