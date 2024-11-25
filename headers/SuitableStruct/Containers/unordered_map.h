/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <SuitableStruct/Internals/DefaultTypes.h>
#include <SuitableStruct/Internals/UniversalInserter.h>
#include <unordered_map>

template<typename... Args>
struct SuitableStruct::ContainerItemType<std::unordered_map<Args...>>
{
    using C = std::unordered_map<Args...>;
    using type = typename std::pair<typename C::key_type, typename C::mapped_type>;
};

template<typename... Args>
struct SuitableStruct::ContainerInserter<std::unordered_map<Args...>>
{
    using C = std::unordered_map<Args...>;
    using Context = SuitableStruct::UniversalInserterContext<C>;
    using value_type = std::pair<typename C::key_type, typename C::mapped_type>; // pair type

    static auto get(C& container) {
        return UniversalInserter(container, [](Context& /*ctx*/, C& container, value_type&& value) {
            container.insert(std::move(value));
        });
    }
};

template<typename... Args> struct SuitableStruct::IsContainer<std::unordered_map<Args...>> : public std::true_type { };
