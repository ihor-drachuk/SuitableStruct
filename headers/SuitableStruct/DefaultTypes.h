#pragma once
#include <type_traits>
#include <SuitableStruct/BufferReader.h>
#include <SuitableStruct/Helpers.h>
#include <SuitableStruct/Handlers.h>

#include <string>

namespace SuitableStruct {

template<typename T,
         typename std::enable_if_t<std::is_fundamental<T>::value>* = nullptr>
Buffer ssSaveImpl(T value)
{
    return Buffer::fromValue(value);
}

template<typename T,
         typename std::enable_if_t<std::is_fundamental<T>::value>* = nullptr>
void ssLoadImpl(BufferReader& buffer, T& value)
{
    buffer.read(value);
}

Buffer ssSaveImpl(const std::string& value);
void ssLoadImpl(BufferReader& buffer, std::string& value);

} // namespace SuitableStruct
