#pragma once
#include <SuitableStruct/Internals/DefaultTypes.h>
#include <list>

template<typename... Args> struct SuitableStruct::IsContainer<std::list<Args...>> : public std::true_type { };
