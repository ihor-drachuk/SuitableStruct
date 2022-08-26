#pragma once
#include <type_traits>
#include <limits>
#include <cstdint>
#include <iterator>
#include <SuitableStruct/Internals/FwdDeclarations.h>
#include <SuitableStruct/Internals/BufferReader.h>
#include <SuitableStruct/Internals/Helpers.h>
#include <SuitableStruct/Internals/Exceptions.h>
#include <SuitableStruct/Handlers.h>
#include <SuitableStruct/Serializer.h>

#include <optional>
#include <string>
#include <memory>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QtContainerFwd>
class QByteArray;
class QString;
class QPoint;
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

template<typename T> struct IsContainer : public std::false_type { };
template<typename T> struct IsAssociativeContainer : public std::false_type { };

template<typename T, typename std::enable_if<can_size<T>::value>::type* = nullptr>
size_t containerSize(const T& container) { return container.size(); }

template<typename T, typename std::enable_if<!can_size<T>::value>::type* = nullptr>
size_t containerSize(const T& container) { return std::distance(container.begin(), container.end()); }

template<typename T>
struct ContainerInserter { static auto get(T& x) { return std::back_inserter(x); } };

Buffer ssSaveImpl(const std::string& value);
void ssLoadImpl(BufferReader& buffer, std::string& value);


#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
template<typename Arg>     struct IsContainer<QVector<Arg>> : public std::true_type { };
template<typename... Args> struct IsContainer<QList<Args...>> : public std::true_type { };
template<>                 struct IsContainer<QStringList> : public std::true_type { };
template<typename... Args> struct IsAssociativeContainer<QMap<Args...>> : public std::true_type { };

Buffer ssSaveImpl(const QByteArray& value);
void ssLoadImpl(BufferReader& buffer, QByteArray& value);
Buffer ssSaveImpl(const QString& value);
void ssLoadImpl(BufferReader& buffer, QString& value);
Buffer ssSaveImpl(const QPoint& value);
void ssLoadImpl(BufferReader& buffer, QPoint& value);
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

template<template<typename, size_t> typename C, typename T, size_t N,
         typename std::enable_if_t<IsContainer<C<T,N>>::value>* = nullptr>
Buffer ssSaveImpl (const C<T,N>& value)
{
    return ssSaveContainerImpl(value);
}

template<typename C>
void ssLoadContainerImpl (BufferReader& buffer, C& value)
{
    using T = typename C::value_type;

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
