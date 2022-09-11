#pragma once

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QJsonValue>
#include <QJsonObject>

#include <cstdint>
#include <type_traits>
#include <tuple>
#include <memory>
#include <limits>
#include <SuitableStruct/Internals/FwdDeclarations.h>
#include <SuitableStruct/Internals/DefaultTypes.h>
#include <SuitableStruct/Internals/Helpers.h>
#include <SuitableStruct/Internals/Exceptions.h>
#include <SuitableStruct/Internals/Version.h>
#include <SuitableStruct/Handlers.h>

namespace SuitableStruct {

// ssJsonSave. Implementation for tuple
template<size_t I = 0, typename... Args, typename... Args2,
         typename std::enable_if<I == sizeof...(Args)>::type* = nullptr>
void ssJsonSaveImplViaTuple(QJsonObject&, const std::tuple<Args...>&, const std::tuple<Args2...>&)
{
}

template<size_t I = 0, typename... Args, typename... Args2,
         typename std::enable_if<!(I >= sizeof...(Args))>::type* = nullptr>
void ssJsonSaveImplViaTuple(QJsonObject& value, const std::tuple<Args...>& args, const std::tuple<Args2...>& names)
{
    value[std::get<I>(names)] = ssJsonSave(std::get<I>(args), false);
    ssJsonSaveImplViaTuple<I+1>(value, args, names);
}

// ssJsonSave.  1) Method
template<typename T,
         typename std::enable_if<can_ssJsonSaveImpl<T>::value>::type* = nullptr>
QJsonValue ssJsonSaveImpl(const T& obj)
{
    return obj.ssJsonSaveImpl();
}

// ssJsonSave.  2) Handlers
template<typename T,
         typename std::enable_if<
             !can_ssJsonSaveImpl<T>::value &&
             ::SuitableStruct::Handlers<T>::value
             >::type* = nullptr>
QJsonValue ssJsonSaveImpl(const T& obj)
{
    return ::SuitableStruct::Handlers<T>::ssJsonSaveImpl(obj);
}

// ssJsonSave.  3) Tuple
template<typename T,
         typename std::enable_if<
             !can_ssJsonSaveImpl<T>::value &&
             !::SuitableStruct::Handlers<T>::value &&
              can_ssTuple<T>::value && can_ssNamesTuple<T>::value
             >::type* = nullptr>
QJsonValue ssJsonSaveImpl(const T& obj)
{
    QJsonObject result;
    ssJsonSaveImplViaTuple(result, obj.ssTuple(), obj.ssNamesTuple());
    return result;
}


template<typename T>
QJsonValue ssJsonSave(const T& obj, bool protectedMode)
{
    QJsonValue part1;
    const auto ver = ssVersion<T>();

    if (ver) {
        QJsonObject contentAndVer;
        contentAndVer["content"] = ssJsonSaveImpl(obj);
        contentAndVer["version"] = *ver;
        part1 = contentAndVer;
    } else {
        part1 = ssJsonSaveImpl(obj);
    }

    if (protectedMode) {
        QJsonValue hashValue;
        auto hash = qHash(part1);

        if (hash <= std::numeric_limits<int>::max()) {
            hashValue = static_cast<int>(hash);
        } else {
            hashValue = QString::number(hash);
        }

        QJsonObject hashWrapper;
        hashWrapper["hash"] = hashValue;
        hashWrapper["content"] = part1;

        return hashWrapper;
    } else {
        return part1;
    }
}


// ssJsonLoad. Implementation for tuple
template<size_t I = 0, typename... Args, typename... Args2,
         typename std::enable_if<I == sizeof...(Args)>::type* = nullptr>
void ssJsonLoadImplViaTuple(const QJsonObject&, std::tuple<Args...>&, const std::tuple<Args2...>&)
{
}

template<size_t I = 0, typename... Args, typename... Args2,
         typename std::enable_if<!(I >= sizeof...(Args))>::type* = nullptr>
void ssJsonLoadImplViaTuple(const QJsonObject& value, std::tuple<Args...>& args, const std::tuple<Args2...>& names)
{
    ssJsonLoad(value[std::get<I>(names)], std::get<I>(args), false);
    ssJsonLoadImplViaTuple<I+1>(value, args, names);
}

// ssJsonLoad.  1) Method
template<typename T,
         typename std::enable_if<can_ssJsonLoadImpl<T&, BufferReader&>::value>::type* = nullptr>
void ssJsonLoadImpl(const QJsonValue& value, T& obj)
{
    obj.ssJsonLoadImpl(value);
}

// ssJsonLoad.  2) Handlers
template<typename T,
         typename std::enable_if<
             !can_ssJsonLoadImpl<T&, BufferReader&>::value &&
             ::SuitableStruct::Handlers<T>::value
             >::type* = nullptr>
void ssJsonLoadImpl(const QJsonValue& value, T& obj)
{
    ::SuitableStruct::Handlers<T>::ssJsonLoadImpl(value, obj);
}

// ssJsonLoad.  3) Tuple
template<typename T,
         typename std::enable_if<
             !can_ssJsonLoadImpl<T&, BufferReader&>::value &&
             !::SuitableStruct::Handlers<T>::value &&
              can_ssTuple<T>::value && can_ssNamesTuple<T>::value
             >::type* = nullptr>
void ssJsonLoadImpl(const QJsonValue& value, T& obj)
{
    assert(value.isObject());
    const auto jsonObject = value.toObject();
    ssJsonLoadImplViaTuple(jsonObject, const_cast_tuple(obj.ssTuple()), obj.ssNamesTuple());
}

//-----------
// Load from ssJsonLoad member
template<typename T>
struct Converters;

template<size_t I, typename T, typename T2,
         typename std::enable_if<!(I <= SSVersionDirect<T>::version)>::type* = nullptr>
void ssJsonLoadAndConvertIter2(T&, const T2&)
{
    assert(!"Unexpected control flow!");
}

template<size_t I, typename T, typename T2,
         typename std::enable_if<I == SSVersionDirect<T>::version>::type* = nullptr>
void ssJsonLoadAndConvertIter2(T& obj, const T2& srcObj)
{
    obj.ssConvertFrom(srcObj);
}

template<size_t I, typename T, typename T2,
         typename std::enable_if<!(I >= SSVersionDirect<T>::version)>::type* = nullptr>
void ssJsonLoadAndConvertIter2(T& obj, const T2& srcObj)
{
    using CurrentType = std::tuple_element_t<I, typename T::ssVersions>;
    CurrentType tempObj;
    tempObj.ssConvertFrom(srcObj);
    ssJsonLoadAndConvertIter2<I+1>(obj, tempObj);
}

template<size_t I, typename T,
         typename std::enable_if<!(I <= SSVersionDirect<T>::version)>::type* = nullptr>
void ssJsonLoadAndConvertIter(const QJsonValue&, T&, uint8_t)
{
    assert(!"Unexpected control flow!");
}

template<size_t I, typename T,
         typename std::enable_if<I <= SSVersionDirect<T>::version>::type* = nullptr>
void ssJsonLoadAndConvertIter(const QJsonValue& value, T& obj, uint8_t serializedVer)
{
    using CurrentType = std::tuple_element_t<I, typename T::ssVersions>;
    constexpr auto neededVer = tuple_type_index<typename T::ssVersions, T>::value;

    // Detect and load saved structure
    if (I < serializedVer) {
        // Not this version, go forward
        ssJsonLoadAndConvertIter<I+1>(value, obj, serializedVer);

    } else if (I == serializedVer && I == neededVer) {
        // This is version, which is saved and also it's version currently used in app
        // Just load it
        ssJsonLoadImpl(value, obj);

    } else if (I == serializedVer && I != neededVer) {
        // This is version, which is saved, but app uses newer version

        // Load old version
        CurrentType oldObject;
        ssJsonLoadImpl(value, oldObject);

        // Convert to new version
        ssJsonLoadAndConvertIter2<I+1>(obj, oldObject);
    }
}

template<typename T,
         typename std::enable_if<has_ssVersions_v<T>>::type* = nullptr>
void ssJsonLoadAndConvert(const QJsonValue& value, T& obj, const std::optional<uint8_t>& ver)
{
    if (ver) {
        ssJsonLoadAndConvertIter<0>(value, obj, *ver);
    } else {
        ssJsonLoadImpl(value, obj);
    }
}

template<typename T,
         typename std::enable_if<!has_ssVersions_v<T>>::type* = nullptr>
void ssJsonLoadAndConvert(const QJsonValue& value, T& obj, const std::optional<uint8_t>& ver)
{
    constexpr auto expectZeroVer = std::is_class_v<T>;

    if (expectZeroVer) {
        assert(ver.value() == 0);
    } else {
        assert(!ver);
    }

    ssJsonLoadImpl(value, obj);
}

template<typename T>
void ssJsonLoad(const QJsonValue& value, T& obj, bool protectedMode)
{
    QJsonValue content;

    // Handle hash-validation
    if (protectedMode) {
        if (!value.isObject())
            Internal::throwIntegrity();

        const auto jsonObject = value.toObject();

        const auto jsonContent = jsonObject["content"];
        const auto jsonHash = jsonObject["hash"];

        uint hash;
        if (jsonHash.isDouble()) {
            hash = qRound(jsonHash.toDouble());
        } else if (jsonHash.isString()) {
            hash = jsonHash.toString().toUInt();
        } else {
            Internal::throwIntegrity();
        }

        if (hash != qHash(jsonContent))
            Internal::throwIntegrity();

        content = jsonContent;
    } else {
        content = value;
    }

    // Handle versioning
    std::optional<uint8_t> ver;
    if (ssVersion<T>()) {
        assert(content.isObject());
        const auto jsonObject = content.toObject();
        ver = jsonObject["version"].toInt();
        content = jsonObject["content"];
    }

    // Load
    T temp;
    ssJsonLoadAndConvert(content, temp, ver);
    obj = std::move(temp);
}

template<typename T>
T ssJsonLoadRet(const QJsonValue& value, bool protectedMode = true)
{
    T result;
    ssJsonLoad(value, result, protectedMode);
    return result;
}

template<typename T>
T ssJsonLoadImplRet(const QJsonValue& value)
{
    T result;
    ssJsonLoadImpl(value, result);
    return result;
}

} // namespace SuitableStruct

#endif // #ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
