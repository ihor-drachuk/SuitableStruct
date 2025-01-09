/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <type_traits>
#include <limits>
#include <cstdint>
#include <chrono>
#include <optional>
#include <string>
#include <memory>
#include <variant>

#include <SuitableStruct/Internals/Common.h>
#include <SuitableStruct/Internals/FwdDeclarations.h>
#include <SuitableStruct/BufferReader.h>
#include <SuitableStruct/Internals/Helpers.h>
#include <SuitableStruct/Internals/Exceptions.h>
#include <SuitableStruct/Handlers.h>
#include <SuitableStruct/Serializer.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QtContainerFwd>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QMetaEnum>
class QByteArray;
class QString;
class QPoint;
class QColor;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QStringList;
#endif // QT_VERSION
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

namespace SuitableStruct {

template<typename T,
         typename std::enable_if_t<std::is_fundamental_v<T> || std::is_enum_v<T>>* = nullptr>
Buffer ssSaveImpl(T value)
{
    return Buffer::fromValue(value);
}

template<typename T,
         typename std::enable_if_t<std::is_fundamental_v<T> || std::is_enum_v<T>>* = nullptr>
void ssLoadImpl(BufferReader& buffer, T& value)
{
    buffer.read(value);
}

template<typename T>
Buffer ssSaveImpl(const std::optional<T>& value)
{
    Buffer result;
    result.write(value.has_value());

    if (value.has_value())
        result += ssSave(value.value(), false);

    return result;
}

template<typename T>
void ssLoadImpl(BufferReader& buffer, std::optional<T>& value)
{
    bool hasValue;
    ssLoadImpl(buffer, hasValue);

    if (hasValue) {
        value.emplace();
        ssLoad(buffer, *value, false);
    } else { // Just precaution
        value.reset();
    }
}

inline Buffer ssSaveImpl(const std::monostate& /*value*/)
{
    return {};
}

inline void ssLoadImpl(BufferReader& /*buffer*/, std::monostate& /*value*/)
{ }

template<typename... Ts>
Buffer ssSaveImpl(const std::variant<Ts...>& value)
{
    Buffer result;
    result.write(static_cast<uint8_t>(value.index()));

    std::visit([&result](const auto& x){ result += ssSave(x, false); }, value);

    return result;
}

template<size_t I, typename... Ts>
void ssLoadImplVariant(BufferReader& buffer, std::variant<Ts...>& value, uint8_t readIndex)
{
    if constexpr (I < sizeof...(Ts)) {
        if (I == readIndex) {
            value = ssLoadRet<std::variant_alternative_t<I, std::variant<Ts...>>>(buffer, false);

        } else {
            ssLoadImplVariant<I + 1>(buffer, value, readIndex);
        }
    } else {
        assert(false && "Shouldn't get here!");
    }
}

template<typename... Ts>
void ssLoadImpl(BufferReader& buffer, std::variant<Ts...>& value)
{
    uint8_t index {};
    buffer.read(index);
    assert(index < sizeof...(Ts));
    ssLoadImplVariant<0>(buffer, value, index);
}

template<typename T1, typename T2>
Buffer ssSaveImpl(const std::pair<T1, T2>& value)
{
    Buffer result;
    result += ssSave(value.first, false);
    result += ssSave(value.second, false);
    return result;
}

template<typename T1, typename T2>
void ssLoadImpl(BufferReader& buffer, std::pair<T1, T2>& value)
{
    ssLoad(buffer, value.first, false);
    ssLoad(buffer, value.second, false);
}

template<typename Rep, typename Period>
Buffer ssSaveImpl(const std::chrono::duration<Rep, Period>& value)
{
    return ssSave(value.count(), false);
}

template<typename Rep, typename Period>
void ssLoadImpl(BufferReader& buffer, std::chrono::duration<Rep, Period>& value)
{
    value = std::chrono::duration<Rep, Period>(ssLoadRet<Rep>(buffer, false));
}

template<typename Clock, typename Duration>
Buffer ssSaveImpl(const std::chrono::time_point<Clock, Duration>& value)
{
    return ssSave(std::chrono::time_point_cast<std::chrono::high_resolution_clock::duration>(value).time_since_epoch(), false);
}

template<typename Clock, typename Duration>
void ssLoadImpl(BufferReader& buffer, std::chrono::time_point<Clock, Duration>& value)
{
    using T = std::chrono::high_resolution_clock::duration;
    value = std::chrono::time_point<Clock, Duration>(typename Clock::time_point(ssLoadRet<T>(buffer, false)));
}

template<typename T>
Buffer ssSaveImpl(const std::shared_ptr<T>& value)
{
    Buffer result;
    result.write(!!value);

    if (value)
        result += ssSave(*value, false);

    return result;
}

template<typename T>
void ssLoadImpl(BufferReader& buffer, std::shared_ptr<T>& value)
{
    bool hasValue;
    ssLoadImpl(buffer, hasValue);

    if (hasValue) {
        value = std::make_shared<T>();
        ssLoad(buffer, *value, false);
    } else { // Just precaution
        value.reset();
    }
}

template<typename T>
Buffer ssSaveImpl(const std::unique_ptr<T>& value)
{
    Buffer result;
    result.write(!!value);

    if (value)
        result += ssSave(*value, false);

    return result;
}

template<typename T>
void ssLoadImpl(BufferReader& buffer, std::unique_ptr<T>& value)
{
    bool hasValue;
    ssLoadImpl(buffer, hasValue);

    if (hasValue) {
        value = std::make_unique<T>();
        ssLoad(buffer, *value, false);
    } else { // Just precaution
        value.reset();
    }
}

Buffer ssSaveImpl(const std::string& value);
void ssLoadImpl(BufferReader& buffer, std::string& value);


#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
template<typename Arg>     struct IsContainer<QVector<Arg>> : public std::true_type { };
template<typename... Args> struct IsContainer<QList<Args...>> : public std::true_type { };
template<>                 struct IsContainer<QStringList> : public std::true_type { };

Buffer ssSaveImpl(const QByteArray& value);
void ssLoadImpl(BufferReader& buffer, QByteArray& value);
Buffer ssSaveImpl(const QString& value);
void ssLoadImpl(BufferReader& buffer, QString& value);
Buffer ssSaveImpl(const QPoint& value);
void ssLoadImpl(BufferReader& buffer, QPoint& value);
Buffer ssSaveImpl(const QColor& value);
void ssLoadImpl(BufferReader& buffer, QColor& value);
Buffer ssSaveImpl(const QJsonValue& value);
void ssLoadImpl(BufferReader& buffer, QJsonValue& value);
Buffer ssSaveImpl(const QTimeZone& value);
void ssLoadImpl(BufferReader& buffer, QTimeZone& value);
Buffer ssSaveImpl(const QDate& value);
void ssLoadImpl(BufferReader& buffer, QDate& value);
Buffer ssSaveImpl(const QTime& value);
void ssLoadImpl(BufferReader& buffer, QTime& value);
Buffer ssSaveImpl(const QDateTime& value);
void ssLoadImpl(BufferReader& buffer, QDateTime& value);
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

template<typename C>
Buffer ssSaveContainerImpl (const C& value)
{
    Buffer result;
    auto size = containerSize(value);
    result.write(static_cast<uint64_t>(size));

    for (const auto& x : value)
        result += ssSave(x, false);

    return result;
}

template<typename C,
         typename std::enable_if_t<IsContainer<C>::value>* = nullptr>
Buffer ssSaveImpl (const C& value)
{
    return ssSaveContainerImpl(value);
}

// std::array<T, N>
template<template<typename, size_t> typename C, typename T, size_t N,
         typename std::enable_if_t<IsContainer<C<T,N>>::value>* = nullptr>
Buffer ssSaveImpl (const C<T,N>& value)
{
    return ssSaveContainerImpl(value);
}

template<typename C>
void ssLoadContainerImpl (BufferReader& buffer, C& value)
{
    using T = typename ContainerItemType<C>::type;

    uint64_t sz;
    buffer.read(sz);

    C result;
    auto sIt = ContainerInserter<C>::get(result);

    for (uint64_t i = 0; i < sz; i++) {
        T item;
        ssLoad(buffer, item, false);
        *sIt++ = std::move(item);
    }

    value = std::move(result);
}

template<typename C,
         typename std::enable_if_t<IsContainer<C>::value>* = nullptr>
void ssLoadImpl (BufferReader& buffer, C& value)
{
    ssLoadContainerImpl(buffer, value);
}

// std::array<T, N>
template<template<typename, size_t> typename C, typename T, size_t N,
         typename std::enable_if_t<IsContainer<C<T,N>>::value>* = nullptr>
void ssLoadImpl (BufferReader& buffer, C<T,N>& value)
{
    ssLoadContainerImpl(buffer, value);
}

template<typename... Args>
Buffer ssSaveImpl (const std::tuple<Args...>& value)
{
    Buffer result;
    auto saver = [&result](const auto& x){ result += ssSave(x, false); };
    std::apply([&saver](const auto&... xs){ (saver(xs), ...); }, value);
    return result;
}

template<typename... Args>
void ssLoadImpl (BufferReader& buffer, std::tuple<Args...>& value)
{
    auto loader = [&buffer](auto& x){ ssLoad(buffer, x, false); };
    std::apply([&loader](auto&... xs){ (loader(xs), ...); }, value);
}

} // namespace SuitableStruct


#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY

namespace SuitableStruct {

QJsonValue ssJsonSaveImpl(bool value);
void ssJsonLoadImpl(const QJsonValue& src, bool& dst);

template<typename T,
         typename std::enable_if_t<std::is_integral_v<T>>* = nullptr>
QJsonValue ssJsonSaveImpl(T value)
{
    // Used for hash storing

    if (value <= std::numeric_limits<int>::max() &&
        value >= std::numeric_limits<int>::min())
    {
        return static_cast<int>(value);

    } else {
        return QString::number(value);
    }
}

template<typename T,
         typename std::enable_if_t<std::is_integral_v<T>>* = nullptr>
void ssJsonLoadImpl(const QJsonValue& src, T& dst)
{
    // Used for hash validation, that's why here is possibility to throw exception

    const auto checkLimits = [](auto value) {
        if (value > std::numeric_limits<T>::max() ||
            value < std::numeric_limits<T>::min())
        {
            Internal::throwOutOfRange();
        }
    };

    if (src.isDouble()) {
        const auto dVal = src.toDouble();
        const auto rVal = [dVal](){
            if constexpr (std::is_signed_v<T>) {
                return static_cast<int64_t>(dVal);
            } else {
                return static_cast<uint64_t>(dVal);
            }
        }();

        // Make sure it's actually integer (not floating-point)
        if (!qFuzzyCompare(dVal, static_cast<double>(rVal)))
            Internal::throwIntegrity();

        checkLimits(rVal);

        dst = static_cast<T>(rVal);

    } else if (src.isString()) {
        bool ok;
        const auto srcStr = src.toString();
        const auto base = srcStr.startsWith(QStringLiteral("0x")) || srcStr.startsWith(QStringLiteral("-0x")) ? 16 : 10;
        const auto tempResult = [&srcStr, &ok, base](){
            if constexpr (std::is_signed_v<T>) {
                return srcStr.toLongLong(&ok, base);
            } else {
                return srcStr.toULongLong(&ok, base);
            }
        }();

        if (ok) {
            checkLimits(tempResult);
            dst = static_cast<T>(tempResult);

        } else {
            Internal::throwIntegrity();
        }

    } else {
        Internal::throwIntegrity();
    }
}

template<typename T,
         typename std::enable_if_t<std::is_floating_point_v<T>>* = nullptr>
QJsonValue ssJsonSaveImpl(T value)
{
    return value;
}

template<typename T,
         typename std::enable_if_t<std::is_floating_point_v<T>>* = nullptr>
void ssJsonLoadImpl(const QJsonValue& src, T& dst)
{
    assert(src.isDouble());
    dst = src.toDouble();
}

QJsonValue ssJsonSaveImpl(QChar value);
void ssJsonLoadImpl(const QJsonValue& src, QChar& dst);

QJsonValue ssJsonSaveImpl(const QString& value);
void ssJsonLoadImpl(const QJsonValue& src, QString& dst);

QJsonValue ssJsonSaveImpl(const QByteArray& value);
void ssJsonLoadImpl(const QJsonValue& src, QByteArray& dst);

QJsonValue ssJsonSaveImpl(const std::string& value);
void ssJsonLoadImpl(const QJsonValue& src, std::string& dst);

QJsonValue ssJsonSaveImpl(const QPoint& value);
void ssJsonLoadImpl(const QJsonValue& src, QPoint& dst);

QJsonValue ssJsonSaveImpl(const QColor& value);
void ssJsonLoadImpl(const QJsonValue& src, QColor& dst);

QJsonValue ssJsonSaveImpl(const QJsonValue& value);
void ssJsonLoadImpl(const QJsonValue& src, QJsonValue& dst);

QJsonValue ssJsonSaveImpl(const QJsonObject& value);
void ssJsonLoadImpl(const QJsonValue& src, QJsonObject& dst);

QJsonValue ssJsonSaveImpl(const QJsonArray& value);
void ssJsonLoadImpl(const QJsonValue& src, QJsonArray& dst);

QJsonValue ssJsonSaveImpl(const QTimeZone& value);
void ssJsonLoadImpl(const QJsonValue& src, QTimeZone& value);

QJsonValue ssJsonSaveImpl(const QDate& value);
void ssJsonLoadImpl(const QJsonValue& src, QDate& value);

QJsonValue ssJsonSaveImpl(const QTime& value);
void ssJsonLoadImpl(const QJsonValue& src, QTime& value);

QJsonValue ssJsonSaveImpl(const QDateTime& value);
void ssJsonLoadImpl(const QJsonValue& src, QDateTime& value);

template<typename T,
         typename std::enable_if_t<std::is_enum_v<T> && QtPrivate::IsQEnumHelper<T>::Value>* = nullptr>
QJsonValue ssJsonSaveImpl(T value)
{
    const auto metaenum = QMetaEnum::fromType<T>();
    assert(metaenum.isValid());
    auto str = metaenum.valueToKey(static_cast<int>(value));
    return str ? QJsonValue(str) :
                 ssJsonSaveImpl(static_cast<int64_t>(value));
}

template<typename T,
         typename std::enable_if_t<std::is_enum_v<T> && !QtPrivate::IsQEnumHelper<T>::Value>* = nullptr>
QJsonValue ssJsonSaveImpl(T value)
{
    return ssJsonSaveImpl(static_cast<int64_t>(value));
}

template<typename T,
         typename std::enable_if_t<std::is_enum_v<T> && QtPrivate::IsQEnumHelper<T>::Value>* = nullptr>
void ssJsonLoadImpl(const QJsonValue& src, T& dst)
{
    const auto metaenum = QMetaEnum::fromType<T>();
    assert(metaenum.isValid());

    if (src.isString()) {
        QString value;
        bool ok;
        ssJsonLoadImpl(src, value);
        auto result = metaenum.keyToValue(value.toLatin1().constData(), &ok);

        if (ok) {
            dst = static_cast<T>(result);
        } else {
            int64_t value2;
            ssJsonLoadImpl(src, value2);
            dst = static_cast<T>(value2);
        }

    } else if (src.isDouble()) {
        int64_t value;
        ssJsonLoadImpl(src, value);
        dst = static_cast<T>(value);

    } else {
        assert(false && "Unexpected type!");
    }
}

template<typename T,
         typename std::enable_if_t<std::is_enum_v<T> && !QtPrivate::IsQEnumHelper<T>::Value>* = nullptr>
void ssJsonLoadImpl(const QJsonValue& src, T& dst)
{
    int64_t value{};
    ssJsonLoadImpl(src, value);
    dst = static_cast<T>(value);
}

template<typename C>
QJsonValue ssJsonSaveContainerImpl (const C& value)
{
    QJsonArray result;

    for (const auto& x : value)
        result += ssJsonSave(x, false);

    return result;
}

template<typename C,
         typename std::enable_if_t<IsContainer<C>::value>* = nullptr>
QJsonValue ssJsonSaveImpl (const C& value)
{
    return ssJsonSaveContainerImpl(value);
}

template<template<typename, size_t> typename C, typename T, size_t N,
         typename std::enable_if_t<IsContainer<C<T,N>>::value>* = nullptr>
QJsonValue ssJsonSaveImpl (const C<T,N>& value)
{
    return ssJsonSaveContainerImpl(value);
}

template<typename C>
void ssJsonLoadContainerImpl (const QJsonValue& value, C& result)
{
    assert(value.isArray());

    using T = typename ContainerItemType<C>::type;
    using Size = decltype(value.toArray().size()); // Size is not saved, so we can use native size type

    const auto arr = value.toArray();

    C temp;
    auto sIt = ContainerInserter<C>::get(temp);
    Size sz = arr.size();

    for (Size i = 0; i < sz; i++) {
        T item;
        ssJsonLoad(arr.at(i), item, false);
        *sIt++ = std::move(item);
    }

    result = std::move(temp);
}

template<typename C,
         typename std::enable_if_t<IsContainer<C>::value>* = nullptr>
void ssJsonLoadImpl (const QJsonValue& value, C& result)
{
    ssJsonLoadContainerImpl(value, result);
}

template<template<typename, size_t> typename C, typename T, size_t N,
         typename std::enable_if_t<IsContainer<C<T,N>>::value>* = nullptr>
void ssJsonLoadImpl (const QJsonValue& value, C<T,N>& result)
{
    ssJsonLoadContainerImpl(value, result);
}

template<typename... Args>
QJsonValue ssJsonSaveImpl (const std::tuple<Args...>& value)
{
    QJsonArray result;
    auto saver = [&result](const auto& x){ result += ssJsonSave(x, false); };
    std::apply([&saver](const auto&... xs){ (saver(xs), ...); }, value);
    return result;
}

template<typename... Args>
void ssJsonLoadImpl (const QJsonValue& value, std::tuple<Args...>& result)
{
    assert(value.isArray());
    const auto arr = value.toArray();
    auto it = arr.begin();
    auto loader = [&value, &it](auto& x){ ssJsonLoad(*it++, x, false); };
    std::apply([&loader](auto&... xs){ (loader(xs), ...); }, result);
}

template<template<typename...> typename SmartPtr, typename T, typename... Args>
QJsonValue ssJsonSaveSmartPtrImpl(const SmartPtr<T, Args...>& value)
{
    QJsonObject obj;
    obj["hasValue"] = !!value;

    if (value) {
        obj["content"] = ssJsonSave(*value, false);
    }

    return obj;
}

template<template<typename...> typename SmartPtr, typename T, typename... Args>
void ssJsonLoadSmartPtrImpl(const QJsonValue& src, SmartPtr<T, Args...>& dst)
{
    assert(src.isObject());
    const auto obj = src.toObject();
    assert(obj.contains("hasValue") && obj["hasValue"].isBool());
    const auto hasValue = obj["hasValue"].toBool();

    if (hasValue) {
        assert(obj.contains("content"));
        SmartPtr<T, Args...> temp (new T);
        ssJsonLoad(obj["content"], *temp, false);
        dst = std::move(temp);

    } else {
        dst.reset();
    }
}

template<typename T>
void ssJsonLoadSmartPtrImpl(const QJsonValue& src, std::optional<T>& dst)
{
    assert(src.isObject());
    const auto obj = src.toObject();
    assert(obj.contains("hasValue") && obj["hasValue"].isBool());
    const auto hasValue = obj["hasValue"].toBool();

    if (hasValue) {
        assert(obj.contains("content"));
        dst.emplace();
        ssJsonLoad(obj["content"], *dst, false);

    } else {
        dst.reset();
    }
}

template<typename... Ts> QJsonValue ssJsonSaveImpl(const std::unique_ptr<Ts...>& value) { return ssJsonSaveSmartPtrImpl(value); }
template<typename... Ts> void ssJsonLoadImpl(const QJsonValue& src, std::unique_ptr<Ts...>& dst) { ssJsonLoadSmartPtrImpl(src, dst); }
template<typename... Ts> QJsonValue ssJsonSaveImpl(const std::shared_ptr<Ts...>& value) { return ssJsonSaveSmartPtrImpl(value); }
template<typename... Ts> void ssJsonLoadImpl(const QJsonValue& src, std::shared_ptr<Ts...>& dst) { ssJsonLoadSmartPtrImpl(src, dst); }
template<typename... Ts> QJsonValue ssJsonSaveImpl(const std::optional<Ts...>& value) { return ssJsonSaveSmartPtrImpl(value); }
template<typename... Ts> void ssJsonLoadImpl(const QJsonValue& src, std::optional<Ts...>& dst) { ssJsonLoadSmartPtrImpl(src, dst); }

template<typename T1, typename T2>
QJsonValue ssJsonSaveImpl(const std::pair<T1, T2>& value)
{
    QJsonObject obj;
    obj["first"] = ssJsonSave(value.first, false);
    obj["second"] = ssJsonSave(value.second, false);
    return obj;
}

template<typename T1, typename T2>
void ssJsonLoadImpl(const QJsonValue& src, std::pair<T1, T2>& dst)
{
    assert(src.isObject());
    const auto obj = src.toObject();
    assert(obj.contains("first"));
    ssJsonLoad(obj["first"], dst.first, false);
    assert(obj.contains("second"));
    ssJsonLoad(obj["second"], dst.second, false);
}

inline QJsonValue ssJsonSaveImpl(const std::monostate& /*value*/)
{
    return {};
}

inline void ssJsonLoadImpl(const QJsonValue& /*src*/, std::monostate& /*value*/)
{ }

template<typename... Ts>
QJsonValue ssJsonSaveImpl(const std::variant<Ts...>& value)
{
    QJsonObject result;
    result["index"] = value.index();
    std::visit([&result](const auto& x){ result["value"] = ssJsonSave(x, false); }, value);

    return result;
}

template<size_t I, typename... Ts>
void ssJsonLoadImplVariant(const QJsonValue& src, std::variant<Ts...>& value, uint8_t readIndex)
{
    if constexpr (I < sizeof...(Ts)) {
        if (I == readIndex) {
            std::get<I>(value) = ssLoadRet<std::variant_alternative_t<I, std::variant<Ts...>>>(src, false);

        } else {
            ssJsonLoadImplVariant<I + 1>(src, value, readIndex);
        }
    } else {
        assert(false && "Shouldn't get here!");
    }
}

template<typename... Ts>
void ssJsonLoadImpl(const QJsonValue& src, std::variant<Ts...>& value)
{
    QJsonObject obj = src.toObject();
    assert(obj.contains("index") && obj["index"].isDouble());
    assert(obj.contains("value"));

    uint8_t index {};
    ssJsonLoadImpl(obj.value("index"), index);
    assert(index < sizeof...(Ts));
    ssJsonLoadImplVariant<0>(obj.value("value"), value, index);
}

template<typename Rep, typename Period>
QJsonValue ssJsonSaveImpl(const std::chrono::duration<Rep, Period>& value)
{
    return ssJsonSave(value.count(), false);
}

template<typename Rep, typename Period>
void ssJsonLoadImpl(const QJsonValue& src, std::chrono::duration<Rep, Period>& value)
{
    value = std::chrono::duration<Rep, Period>(ssJsonLoadRet<Rep>(src, false));
}

template<typename Clock, typename Duration>
QJsonValue ssJsonSaveImpl(const std::chrono::time_point<Clock, Duration>& value)
{
    return ssJsonSave(std::chrono::time_point_cast<std::chrono::high_resolution_clock::duration>(value).time_since_epoch(), false);
}

template<typename Clock, typename Duration>
void ssJsonLoadImpl(const QJsonValue& src, std::chrono::time_point<Clock, Duration>& value)
{
    using T = std::chrono::high_resolution_clock::duration;
    value = std::chrono::time_point<Clock, Duration>(typename Clock::time_point(ssJsonLoadRet<T>(src, false)));
}

} // namespace SuitableStruct

#endif // #ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
