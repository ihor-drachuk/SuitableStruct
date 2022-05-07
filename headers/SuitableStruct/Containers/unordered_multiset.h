#pragma once
#include <SuitableStruct/Internals/DefaultTypes.h>
#include <SuitableStruct/Internals/UniversalInserter.h>
#include <unordered_set>

template<typename... Args>
struct SuitableStruct::ContainerInserter<std::unordered_multiset<Args...>>
{
    using C = std::unordered_multiset<Args...>;
    using Context = SuitableStruct::UniversalInserterContext<C>;
    using value_type = typename C::value_type;

    static auto get(C& container) {
        return UniversalInserter(container, [](Context& ctx, C& container, const value_type& value) {
            container.insert(value);
        });
    }
};

template<typename... Args> struct SuitableStruct::IsContainer<std::unordered_multiset<Args...>> : public std::true_type { };
