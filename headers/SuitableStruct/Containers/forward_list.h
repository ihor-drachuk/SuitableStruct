/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <SuitableStruct/Internals/Common.h>
#include <SuitableStruct/Internals/UniversalInserter.h>
#include <forward_list>

template<typename... Args>
struct SuitableStruct::UniversalInserterContext<std::forward_list<Args...>>
{
    typename std::forward_list<Args...>::iterator lastIt;
};

template<typename... Args>
struct SuitableStruct::ContainerInserter<std::forward_list<Args...>>
{
    using C = std::forward_list<Args...>;
    using Context = SuitableStruct::UniversalInserterContext<std::forward_list<Args...>>;
    using value_type = typename C::value_type;

    static auto get(C& container) {
        return UniversalInserter(container, [](Context& ctx, C& container, const value_type& value) {
            container.insert_after(ctx.lastIt, value);
            ++ctx.lastIt;
        },
        [](Context& ctx, C& container){
            auto beforeEnd = container.before_begin();
            for (auto& _ : container)
                ++beforeEnd;

            ctx.lastIt = beforeEnd;
        });
    }
};

template<typename... Args> struct SuitableStruct::IsContainer<std::forward_list<Args...>> : public std::true_type { };
