/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <tuple>
#include <type_traits>
#include <SuitableStruct/Internals/Helpers.h>
#include <SuitableStruct/Handlers.h>


namespace SuitableStruct {

// Check if type has ssVersions in itself
template<typename T, typename = void>
struct HasSSVersionsInType : std::false_type {};

template<typename T>
struct HasSSVersionsInType<T, std::void_t<typename T::ssVersions>> : std::true_type {};

// Check if type has ssVersions in its Handlers
template<typename T, typename = void>
struct HasSSVersionsInHandlers : std::false_type {};

template<typename T>
struct HasSSVersionsInHandlers<T, std::void_t<typename Handlers<T>::ssVersions>> : std::true_type {};

// New type traits for versioning checks
template<typename T>
struct HasSSVersionsInTypeOrHandlers {
    static constexpr bool value = HasSSVersionsInType<T>::value || HasSSVersionsInHandlers<T>::value;
};

// Get ssVersions from type or its Handlers
template<typename T, typename = void>
struct SSVersions
{
    using type = std::tuple<T>;
};

template<typename T>
struct SSVersions<T, std::enable_if_t<HasSSVersionsInType<T>::value>>
{
    using type = typename T::ssVersions;
};

template<typename T>
struct SSVersions<T, std::enable_if_t<!HasSSVersionsInType<T>::value && HasSSVersionsInHandlers<T>::value>>
{
    using type = typename Handlers<T>::ssVersions;
};

template<typename T>
using SSVersions_t = typename SSVersions<T>::type;

template<typename T>
struct SSVersion
{
    static constexpr auto value = std::tuple_size_v<SSVersions_t<T>> - 1;
};

} // namespace SuitableStruct
