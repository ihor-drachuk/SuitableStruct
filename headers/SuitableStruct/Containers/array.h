#pragma once
#include <SuitableStruct/Internals/DefaultTypes.h>
#include <array>

template<typename Arg, size_t N>
struct SuitableStruct::ContainerInserter<std::array<Arg,N>> { static auto get(std::array<Arg,N>& x) { return std::begin(x); } };

template<typename Arg, size_t N>
struct SuitableStruct::IsContainer<std::array<Arg, N>> : public std::true_type { };
