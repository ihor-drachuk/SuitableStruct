#pragma once
#include <SuitableStruct/Internals/DefaultTypes.h>
#include <deque>

template<typename... Args> struct SuitableStruct::IsContainer<std::deque<Args...>> : public std::true_type { };
