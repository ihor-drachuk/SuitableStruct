/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <type_traits>

namespace SuitableStruct {

template<typename T>
struct Handlers : public std::false_type
{ };

} // namespace SuitableStruct
