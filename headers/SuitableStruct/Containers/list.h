/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <SuitableStruct/Internals/Common.h>
#include <list>

template<typename... Args> struct SuitableStruct::IsContainer<std::list<Args...>> : public std::true_type { };
