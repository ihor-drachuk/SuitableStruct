#pragma once
#include <SuitableStruct/Internals/DefaultTypes.h>
#include <SuitableStruct/Internals/UniversalInserter.h>
#include <QSet>

template<typename... Args>
struct SuitableStruct::ContainerInserter<QSet<Args...>>
{
    using C = QSet<Args...>;
    using Context = SuitableStruct::UniversalInserterContext<C>;
    using value_type = typename C::value_type;

    static auto get(C& container) {
        return UniversalInserter(container, [](Context& ctx, C& container, const value_type& value) {
            container.insert(value);
        });
    }
};

template<typename... Args> struct SuitableStruct::IsContainer<QSet<Args...>> : public std::true_type { };
