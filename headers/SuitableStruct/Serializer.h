/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <cstdint>
#include <type_traits>
#include <tuple>
#include <limits>
#include <SuitableStruct/Internals/FwdDeclarations.h>
#include <SuitableStruct/Internals/DefaultTypes.h>
#include <SuitableStruct/Internals/Helpers.h>
#include <SuitableStruct/Internals/Exceptions.h>
#include <SuitableStruct/Internals/Version.h>
#include <SuitableStruct/Buffer.h>
#include <SuitableStruct/Handlers.h>


// Features:
//   - Serialization / deserialization for structures
//   - Data integrity validation
//   - Versioning

// TODO:
//  - SSO-replacement for std::optional

namespace SuitableStruct {

// Versions
template<typename T>
void ssWriteVersion(Buffer& buf)
{
    if (const auto ver = ssVersion<T>())
        buf.write(*ver);
};

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

// ssSave. Implementation for tuple
template<size_t I = 0, typename... Args,
         typename std::enable_if<I == sizeof...(Args)>::type* = nullptr>
void ssSaveImplViaTuple(Buffer&, const std::tuple<Args...>&)
{
}

template<size_t I = 0, typename... Args,
         typename std::enable_if<!(I >= sizeof...(Args))>::type* = nullptr>
void ssSaveImplViaTuple(Buffer& buffer, const std::tuple<Args...>& args)
{
    buffer += ssSave(std::get<I>(args), false);
    ssSaveImplViaTuple<I+1>(buffer, args);
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
    ssSaveImplViaTuple(buf, obj.ssTuple());
    return buf;
}


template<typename T>
Buffer ssSave(const T& obj, bool protectedMode /*= true*/)
{
    Buffer result;

    if (protectedMode) {
        Buffer part;
        part.writeZeros(5); // Global reserved
        ssWriteVersion<T>(part);
        ssBeforeSaveImpl(obj);
        part += ssSaveImpl(obj);
        ssAfterSaveImpl(obj);

        static_assert (sizeof(part.hash()) == sizeof(uint32_t), "Make sure save & load expect same type!");
        result.write(static_cast<uint64_t>(part.size()));
        result.write(part.hash());
        result += part;
    } else {
        ssWriteVersion<T>(result);
        ssBeforeSaveImpl(obj);
        result += ssSaveImpl(obj);
        ssAfterSaveImpl(obj);
    }

    return result;
}


// ssLoad. Implementation for tuple
template<size_t I = 0, typename... Args,
         typename std::enable_if<I == sizeof...(Args)>::type* = nullptr>
void ssLoadImplViaTuple(BufferReader&, std::tuple<Args...>&)
{
}

template<size_t I = 0, typename... Args,
         typename std::enable_if<!(I >= sizeof...(Args))>::type* = nullptr>
void ssLoadImplViaTuple(BufferReader& buffer, std::tuple<Args...>& args)
{
    ssLoad(buffer, std::get<I>(args), false);
    ssLoadImplViaTuple<I+1>(buffer, args);
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
    ssLoadImplViaTuple(buf, const_cast_tuple(obj.ssTuple()));
}

//-----------
// Load from ssLoad member
template<typename T>
struct Converters;

template<size_t I, typename T, typename T2,
         typename std::enable_if<!(I <= SSVersionDirect<T>::version)>::type* = nullptr>
void ssLoadAndConvertIter2(T&, T2&&)
{
    assert(false && "Unexpected control flow!");
}

template<size_t I, typename T, typename T2,
         typename std::enable_if<I == SSVersionDirect<T>::version>::type* = nullptr>
void ssLoadAndConvertIter2(T& obj, T2&& srcObj)
{
    obj.ssConvertFrom(std::forward<T2&&>(srcObj));
}

template<size_t I, typename T, typename T2,
         typename std::enable_if<!(I >= SSVersionDirect<T>::version)>::type* = nullptr>
void ssLoadAndConvertIter2(T& obj, T2&& srcObj)
{
    using CurrentType = std::tuple_element_t<I, typename T::ssVersions>;
    auto tempObj = construct<CurrentType>();
    tempObj.ssConvertFrom(std::forward<T2&&>(srcObj));
    ssLoadAndConvertIter2<I+1>(obj, std::move(tempObj));
}

template<size_t I, typename T,
         typename std::enable_if<!(I <= SSVersionDirect<T>::version)>::type* = nullptr>
void ssLoadAndConvertIter(BufferReader&, T&, uint8_t)
{
    assert(false && "Unexpected control flow!");
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
        auto oldObject = construct<CurrentType>();
        ssLoadImpl(buffer, oldObject);

        // Convert to new version
        ssLoadAndConvertIter2<I+1>(obj, std::move(oldObject));
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
void ssLoad(BufferReader& buffer, T& obj, bool protectedMode /*= true*/)
{
    if (protectedMode) {
        auto temp = construct<T>();

        uint64_t size;
        decltype(std::declval<Buffer>().hash()) /*uint32*/ hash;
        buffer.read(size);
        buffer.read(hash);

        if (size > std::numeric_limits<size_t>::max())
            Internal::throwTooLarge();

        auto groupData = buffer.readRaw(size);
        const auto actualHash = groupData.hash();

        if (hash != actualHash)
            Internal::throwIntegrity();

        groupData.advance(5); // Reserved
        auto ver = ssReadVersion<T>(groupData);
        ssBeforeLoadImpl(temp);
        ssLoadAndConvert(groupData, temp, ver);
        ssAfterLoadImpl(temp);
        obj = std::move(temp);

    } else {
        auto ver = ssReadVersion<T>(buffer);
        ssBeforeLoadImpl(obj);
        ssLoadAndConvert(buffer, obj, ver);
        ssAfterLoadImpl(obj);
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
    ssLoad(BufferReader(buffer), obj, protectedMode);
}

template<typename T>
[[nodiscard]] T ssLoadRet(BufferReader& reader, bool protectedMode /*= true*/)
{
    auto result = construct<T>();
    ssLoad(reader, result, protectedMode);
    return result;
}

template<typename T>
[[nodiscard]] T ssLoadRet(BufferReader&& reader, bool protectedMode = true)
{
    return ssLoadRet<T>(static_cast<BufferReader&>(reader), protectedMode);
}

template<typename T>
[[nodiscard]] T ssLoadRet(const Buffer& buffer, bool protectedMode = true)
{
    return ssLoadRet<T>(BufferReader(buffer), protectedMode);
}

template<typename T>
[[nodiscard]] T ssLoadImplRet(BufferReader& reader)
{
    auto result = construct<T>();
    ssLoadImpl(reader, result);
    return result;
}

template<typename T>
[[nodiscard]] T ssLoadImplRet(BufferReader&& reader)
{
    return ssLoadImplRet<T>(static_cast<BufferReader&>(reader));
}

} // namespace SuitableStruct
