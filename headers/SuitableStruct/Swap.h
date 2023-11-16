/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <tuple>
#include <utility>
#include <SuitableStruct/Internals/Helpers.h>

namespace SuitableStructInternal {

template<typename T>
void ssSwap(T& first, T& second)
{
    std::swap(first, second);
}

template<typename Struct, typename T>
void ssSwap(const Struct&, T& first, T& second)
{
    ssSwap(first, second);
}

template<typename Struct, std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
swapTuples(const Struct&, std::tuple<Tp...>&, std::tuple<Tp...>&)
{
}

template<typename Struct, std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), void>::type
swapTuples(const Struct& s, std::tuple<Tp...>& t1, std::tuple<Tp...>& t2)
{
    ssSwap(s, std::get<I>(t1), std::get<I>(t2));
    swapTuples<Struct, I + 1, Tp...>(s, t1, t2);
}

} // namespace SuitableStructInternal

#define SS_SWAP_OP(STRUCT) \
    inline void ssSwap(STRUCT& lhs, STRUCT& rhs) \
    { \
        using namespace SuitableStruct; \
        SuitableStructInternal::swapTuples(lhs, const_cast_tuple(lhs.ssTuple()), const_cast_tuple(rhs.ssTuple())); \
    }

#define SS_SWAP(STRUCT) \
    SS_SWAP_OP(STRUCT)
