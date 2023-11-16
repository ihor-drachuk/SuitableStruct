/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <SuitableStruct/Internals/DefaultTypes.h>
#include <deque>

template<typename... Args> struct SuitableStruct::IsContainer<std::deque<Args...>> : public std::true_type { };
