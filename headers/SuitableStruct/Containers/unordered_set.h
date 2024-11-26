/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <SuitableStruct/Internals/Common.h>
#include <SuitableStruct/Internals/UniversalInserter.h>
#include <unordered_set>

template<typename... Args>
struct SuitableStruct::ContainerInserter<std::unordered_set<Args...>>
{
    using C = std::unordered_set<Args...>;
    using Context = SuitableStruct::UniversalInserterContext<C>;
    using value_type = typename C::value_type;

    static auto get(C& container) {
        return UniversalInserter(container, [](Context& /*ctx*/, C& container, const value_type& value) {
            container.insert(value);
        });
    }
};

template<typename... Args> struct SuitableStruct::IsContainer<std::unordered_set<Args...>> : public std::true_type { };
