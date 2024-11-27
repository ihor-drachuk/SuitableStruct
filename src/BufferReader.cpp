/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <SuitableStruct/BufferReader.h>
#include <SuitableStruct/Internals/Exceptions.h>
#include <SuitableStruct/Hashes.h>

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
