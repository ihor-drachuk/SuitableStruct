/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <SuitableStruct/Internals/DefaultTypes.h>
#include <SuitableStruct/Internals/UniversalInserter.h>
#include <set>

template<typename... Args>
struct SuitableStruct::ContainerInserter<std::multiset<Args...>>
{
    using C = std::multiset<Args...>;
    using Context = SuitableStruct::UniversalInserterContext<C>;
    using value_type = typename C::value_type;

    static auto get(C& container) {
        return UniversalInserter(container, [](Context& /*ctx*/, C& container, const value_type& value) {
            container.insert(value);
        });
    }
};

template<typename... Args> struct SuitableStruct::IsContainer<std::multiset<Args...>> : public std::true_type { };
