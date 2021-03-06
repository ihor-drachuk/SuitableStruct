#pragma once
#include <stdexcept>
#include <cstdint>
#include <type_traits>
#include <tuple>
#include <memory>
#include <limits>
#include <SuitableStruct/Buffer.h>
#include <SuitableStruct/DefaultTypes.h>
#include <SuitableStruct/Helpers.h>
#include <SuitableStruct/Handlers.h>
#include <SuitableStruct/LongSSO.h>

// Features:
//   - Serialization / deserialization for structures
//   - Data integrity validation
//   - Hashing
//   - Versioning
//   - Copy-and-swap helper

// TODO:
//  - SSO-replacement for std::optional

namespace SuitableStruct {

// Versions
template<typename T,
         typename std::enable_if<!has_ssVersions_v<T> && std::is_class_v<T>>::type* = nullptr>
void ssWriteVersion(Buffer& buf)
{
    buf.write((uint8_t)0);
};

template<typename T,
         typename std::enable_if<!has_ssVersions_v<T> && !std::is_class_v<T>>::type* = nullptr>
void ssWriteVersion(Buffer&)
{
    // Nothing.
};

template<typename T,
         typename std::enable_if<has_ssVersions_v<T>>::type* = nullptr>
void ssWriteVersion(Buffer& buf)
{
    constexpr auto verIndex = tuple_type_index<typename T::ssVersions, T>::value;
    static_assert (verIndex >= 0, "Something wrong with version detection");
    buf.write((uint8_t)verIndex);
}

template<typename T,
         typename std::enable_if<!has_ssVersions_v<T> && !std::is_class_v<T>>::type* = nullptr>
std::optional<uint8_t> ssReadVersion(BufferReader&) { return {}; };

template<typename T,
         typename std::enable_if<!has_ssVersions_v<T> && std::is_class_v<T>>::type* = nullptr>
std::optional<uint8_t> ssReadVersion(BufferReader& buf)
{
    uint8_t result;
    buf.read(result);
    assert(result == 0);
    return result;
};

template<typename T,
         typename std::enable_if<has_ssVersions_v<T>>::type* = nullptr>
std::optional<uint8_t> ssReadVersion(BufferReader& buf)
{
    uint8_t result;
    buf.read(result);
    return result;
}

template<typename T>
struct SSVersionDirect
{
    static constexpr auto version = std::tuple_size_v<typename T::ssVersions> - 1;
};

template<typename T,
         typename std::enable_if<has_ssVersions_v<T>>::type* = nullptr>
std::optional<uint8_t> ssVersion()
{
    return std::tuple_size_v<typename T::ssVersions> - 1;
}

template<typename T,
         typename std::enable_if<!has_ssVersions_v<T> && !std::is_class_v<T>>::type* = nullptr>
std::optional<uint8_t> ssVersion()
{
    return {};
}

template<typename T,
         typename std::enable_if<!has_ssVersions_v<T> && std::is_class_v<T>>::type* = nullptr>
std::optional<uint8_t> ssVersion()
{
    return 0;
}

// ssSave. Forward
template<typename T>
Buffer ssSave(const T& obj, bool protectedMode = true);

// ssSave. Implementation for tuple
template<size_t I = 0, typename... Args,
         typename std::enable_if<I == sizeof...(Args)>::type* = nullptr>
void ssSaveImpl(Buffer&, const std::tuple<Args...>&)
{
}

template<size_t I = 0, typename... Args,
         typename std::enable_if<!(I >= sizeof...(Args))>::type* = nullptr>
void ssSaveImpl(Buffer& buffer, const std::tuple<Args...>& args)
{
    buffer += ssSave(std::get<I>(args), false);
    ssSaveImpl<I+1>(buffer, args);
}

// ssSave.  1) Method
template<typename T,
         typename std::enable_if<can_ssSaveImpl<T>::value>::type* = nullptr>
Buffer ssSaveImpl(const T& obj)
{
    return obj.ssSaveImpl();
}

// ssSave.  2) Handlers
template<typename T,
         typename std::enable_if<
             !can_ssSaveImpl<T>::value &&
             ::SuitableStruct::Handlers<T>::value
             >::type* = nullptr>
Buffer ssSaveImpl(const T& obj)
{
    return ::SuitableStruct::Handlers<T>::ssSaveImpl(obj);
}

// ssSave.  3) Tuple
template<typename T,
         typename std::enable_if<
             !can_ssSaveImpl<T>::value &&
             !::SuitableStruct::Handlers<T>::value &&
              can_ssTuple<T>::value
             >::type* = nullptr>
Buffer ssSaveImpl(const T& obj)
{
    Buffer buf;
    ssSaveImpl(buf, obj.ssTuple());
    return buf;
}


template<typename T>
Buffer ssSave(const T& obj, bool protectedMode)
{
    Buffer result;

    if (protectedMode) {
        Buffer part;
        ssWriteVersion<T>(part);
        part += ssSaveImpl(obj);

        static_assert (sizeof(part.hash() == sizeof(uint32_t)), "Make sure save & load expect same type!");
        result.write((uint64_t)part.size());
        result.write(part.hash());
        result += part;
    } else {
        ssWriteVersion<T>(result);
        result += ssSaveImpl(obj);
    }

    return result;
}


// ssLoad. Implementation for tuple
template<size_t I = 0, typename... Args,
         typename std::enable_if<I == sizeof...(Args)>::type* = nullptr>
void ssLoadImpl(BufferReader&, std::tuple<Args...>&)
{
}

template<size_t I = 0, typename... Args,
         typename std::enable_if<!(I >= sizeof...(Args))>::type* = nullptr>
void ssLoadImpl(BufferReader& buffer, std::tuple<Args...>& args)
{
    ssLoad(buffer, std::get<I>(args), false);
    ssLoadImpl<I+1>(buffer, args);
}

// ssLoad.  1) Method
template<typename T,
         typename std::enable_if<can_ssLoadImpl<T&, BufferReader&>::value>::type* = nullptr>
void ssLoadImpl(BufferReader& buf, T& obj)
{
    obj.ssLoadImpl(buf);
}

// ssLoad.  2) Handlers
template<typename T,
         typename std::enable_if<
             !can_ssLoadImpl<T&, BufferReader&>::value &&
             ::SuitableStruct::Handlers<T>::value
             >::type* = nullptr>
void ssLoadImpl(BufferReader& buf, T& obj)
{
    ::SuitableStruct::Handlers<T>::ssLoadImpl(buf, obj);
}

// ssLoad.  3) Tuple
template<typename T,
         typename std::enable_if<
             !can_ssLoadImpl<T&, BufferReader&>::value &&
             !::SuitableStruct::Handlers<T>::value &&
              can_ssTuple<T>::value
             >::type* = nullptr>
void ssLoadImpl(BufferReader& buf, T& obj)
{
    ssLoadImpl(buf, const_cast_tuple(obj.ssTuple()));
}

//-----------
// Load from ssLoad member
template<typename T>
struct Converters;

template<size_t I, typename T, typename T2,
         typename std::enable_if<!(I <= SSVersionDirect<T>::version)>::type* = nullptr>
void ssLoadAndConvertIter2(T&, const T2&)
{
    assert(!"Unexpected control flow!");
}

template<size_t I, typename T, typename T2,
         typename std::enable_if<I == SSVersionDirect<T>::version>::type* = nullptr>
void ssLoadAndConvertIter2(T& obj, const T2& srcObj)
{
    obj.ssConvertFrom(srcObj);
}

template<size_t I, typename T, typename T2,
         typename std::enable_if<!(I >= SSVersionDirect<T>::version)>::type* = nullptr>
void ssLoadAndConvertIter2(T& obj, const T2& srcObj)
{
    using CurrentType = std::tuple_element_t<I, typename T::ssVersions>;
    CurrentType tempObj;
    tempObj.ssConvertFrom(srcObj);
    ssLoadAndConvertIter2<I+1>(obj, tempObj);
}

template<size_t I, typename T,
         typename std::enable_if<!(I <= SSVersionDirect<T>::version)>::type* = nullptr>
void ssLoadAndConvertIter(BufferReader&, T&, uint8_t)
{
    assert(!"Unexpected control flow!");
}

template<size_t I, typename T,
         typename std::enable_if<I <= SSVersionDirect<T>::version>::type* = nullptr>
void ssLoadAndConvertIter(BufferReader& buffer, T& obj, uint8_t serializedVer)
{
    using CurrentType = std::tuple_element_t<I, typename T::ssVersions>;
    constexpr auto neededVer = tuple_type_index<typename T::ssVersions, T>::value;

    // Detect and load saved structure
    if (I < serializedVer) {
        // Not this version, go forward
        ssLoadAndConvertIter<I+1>(buffer, obj, serializedVer);

    } else if (I == serializedVer && I == neededVer) {
        // This is version, which is saved and also it's version currently used in app
        // Just load it
        ssLoadImpl(buffer, obj);

    } else if (I == serializedVer && I != neededVer) {
        // This is version, which is saved, but app uses newer version

        // Load old version
        CurrentType oldObject;
        ssLoadImpl(buffer, oldObject);

        // Convert to new version
        ssLoadAndConvertIter2<I+1>(obj, oldObject);
    }
}

template<typename T,
         typename std::enable_if<has_ssVersions_v<T>>::type* = nullptr>
void ssLoadAndConvert(BufferReader& buffer, T& obj, const std::optional<uint8_t>& ver)
{
    if (ver) {
        ssLoadAndConvertIter<0>(buffer, obj, *ver);
    } else {
        ssLoadImpl(buffer, obj);
    }
}

template<typename T,
         typename std::enable_if<!has_ssVersions_v<T>>::type* = nullptr>
void ssLoadAndConvert(BufferReader& buffer, T& obj, const std::optional<uint8_t>& ver)
{
    constexpr auto expectZeroVer = std::is_class_v<T>;

    if (expectZeroVer) {
        assert(ver.value() == 0);
    } else {
        assert(!ver);
    }

    ssLoadImpl(buffer, obj);
}

template<typename T>
void ssLoad(BufferReader& buffer, T& obj, bool protectedMode = true)
{
    if (protectedMode) {
        uint64_t size;
        decltype(std::declval<Buffer>().hash()) /*uint32*/ hash;
        buffer.read(size);
        buffer.read(hash);

        if (size > std::numeric_limits<size_t>::max())
            throw std::runtime_error("Can't load! Buffer is too large!");

        auto groupData = buffer.readRaw(size);
        const auto actualHash = groupData.hash();

        if (hash != actualHash)
            throw std::runtime_error("Integrity check failed!");

        auto ver = ssReadVersion<T>(groupData);
        ssLoadAndConvert(groupData, obj, ver);

    } else {
        auto ver = ssReadVersion<T>(buffer);
        ssLoadAndConvert(buffer, obj, ver);
    }
}

template<typename T>
void ssLoad(BufferReader&& reader, T& obj, bool protectedMode = true)
{
    ssLoad(static_cast<BufferReader&>(reader), obj, protectedMode);
}

template<typename T>
void ssLoad(const Buffer& buffer, T& obj, bool protectedMode = true)
{
    BufferReader reader(buffer);
    ssLoad(reader, obj, protectedMode);
}

} // namespace SuitableStruct
