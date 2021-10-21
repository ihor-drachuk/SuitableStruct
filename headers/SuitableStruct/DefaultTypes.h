#pragma once
#include <type_traits>
#include <limits>
#include <cstdint>
#include <iterator>
#include <SuitableStruct/BufferReader.h>
#include <SuitableStruct/Helpers.h>
#include <SuitableStruct/Handlers.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Exceptions.h>

#include <string>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
class QByteArray;
class QString;
template <typename T> class QVector;
template <typename T> class QList;
template <class Key, class T> class QMap;
#endif

namespace SuitableStruct {

template<typename T,
         typename std::enable_if_t<std::is_fundamental<T>::value>* = nullptr>
Buffer ssSaveImpl(T value)
{
    return Buffer::fromValue(value);
}

template<typename T,
         typename std::enable_if_t<std::is_fundamental<T>::value>* = nullptr>
void ssLoadImpl(BufferReader& buffer, T& value)
{
    buffer.read(value);
}

template<typename T> struct IsContainer : public std::false_type { };
template<typename T> struct IsAssociativeContainer : public std::false_type { };


Buffer ssSaveImpl(const std::string& value);
void ssLoadImpl(BufferReader& buffer, std::string& value);

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
template<typename... Args> struct IsContainer<QVector<Args...>> : public std::true_type { };
template<typename... Args> struct IsContainer<QList<Args...>> : public std::true_type { };
template<typename... Args> struct IsAssociativeContainer<QMap<Args...>> : public std::true_type { };

Buffer ssSaveImpl(const QByteArray& value);
void ssLoadImpl(BufferReader& buffer, QByteArray& value);
Buffer ssSaveImpl(const QString& value);
void ssLoadImpl(BufferReader& buffer, QString& value);

template<template<typename, typename...> typename C, typename T, typename... Args,
         typename std::enable_if_t<IsContainer<C<T,Args...>>::value>* = nullptr>
Buffer ssSaveImpl (const C<T,Args...>& value)
{
    Buffer result;
    result.write((uint64_t)value.size());

    for (const auto& x : value)
        result += ssSave(x, false);

    return result;
}

template<template<typename, typename...> typename C, typename T, typename... Args,
         typename std::enable_if_t<IsContainer<C<T,Args...>>::value>* = nullptr>
void ssLoadImpl (BufferReader& buffer, C<T,Args...>& value)
{
    using Size = decltype (value.size());

    uint64_t sz;
    buffer.read(sz);

    if (sz > std::numeric_limits<Size>::max())
        Internal::throwTooMany();

    C<T,Args...> result;
    auto sIt = std::back_inserter(result);

    for (Size i = 0; i < sz; i++) {
        T item;
        ssLoad(buffer, item, false);
        *sIt = std::move(item);
    }

    value = std::move(result);
}
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

} // namespace SuitableStruct
