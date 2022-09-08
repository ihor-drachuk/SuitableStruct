#pragma once
#include <optional>
#include <tuple>
#include <type_traits>
#include <SuitableStruct/Internals/Helpers.h>


namespace SuitableStruct {

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

} // namespace SuitableStruct
