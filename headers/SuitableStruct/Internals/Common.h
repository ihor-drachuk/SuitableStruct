/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <type_traits>
#include <iterator>
#include <SuitableStruct/Internals/Helpers.h>

namespace SuitableStruct {

template<typename T> struct IsContainer : public std::false_type { };

template<typename T, typename std::enable_if<can_size<T>::value>::type* = nullptr>
size_t containerSize(const T& container) { return container.size(); }

template<typename T, typename std::enable_if<!can_size<T>::value>::type* = nullptr>
size_t containerSize(const T& container) { return std::distance(container.begin(), container.end()); }

template<typename T>
struct ContainerInserter { static auto get(T& x) { return std::back_inserter(x); } };

template<typename T>
struct ContainerItemType { using type = typename T::value_type; };

} // namespace SuitableStruct
