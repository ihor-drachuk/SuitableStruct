#include <SuitableStruct/BufferReader.h>
#include <SuitableStruct/Hashes.h>
#include <stdexcept>

namespace SuitableStruct {

uint32_t BufferReader::hash() const
{
    const auto sz = size();
    return sz ? ssHashRaw(data(), sz) : 0;
}

void BufferReader::checkPosition(size_t sz) const
{
    if (position() + sz > size())
        throw std::out_of_range("Out of range");
}

} // namespace SuitableStruct
