#include <SuitableStruct/BufferReader.h>
#include <SuitableStruct/Hashes.h>
#include <SuitableStruct/Exceptions.h>

namespace SuitableStruct {

uint32_t BufferReader::hash() const
{
    const auto sz = size();
    return sz ? ssHashRaw(data(), sz) : 0;
}

void BufferReader::checkPosition(size_t sz) const
{
    if (position() + sz > size())
        Internal::throwOutOfRange();
}

} // namespace SuitableStruct
