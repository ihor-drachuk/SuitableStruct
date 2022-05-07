#pragma once
#include <SuitableStruct/Internals/DefaultTypes.h>
#include <vector>

template<typename... Args> struct SuitableStruct::IsContainer<std::vector<Args...>> : public std::true_type { };
