/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <SuitableStruct/Internals/Common.h>
#include <SuitableStruct/Internals/UniversalInserter.h>
#include <map>

template<typename... Args>
struct SuitableStruct::ContainerItemType<std::map<Args...>>
{
    using C = std::map<Args...>;
    using type = typename std::pair<typename C::key_type, typename C::mapped_type>;
};

template<typename... Args>
struct SuitableStruct::ContainerInserter<std::map<Args...>>
{
    using C = std::map<Args...>;
    using Context = SuitableStruct::UniversalInserterContext<C>;
    using value_type = std::pair<typename C::key_type, typename C::mapped_type>; // pair type

    static auto get(C& container) {
        return UniversalInserter(container, [](Context& /*ctx*/, C& container, value_type&& value) {
            container.insert(std::move(value));
        });
    }
};

template<typename... Args> struct SuitableStruct::IsContainer<std::map<Args...>> : public std::true_type { };
