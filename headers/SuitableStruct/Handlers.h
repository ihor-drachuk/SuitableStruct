#pragma once
#include <tuple>
#include <type_traits>

namespace SuitableStruct {

template<typename T>
struct Handlers : public std::false_type
{ };

} // namespace SuitableStruct
