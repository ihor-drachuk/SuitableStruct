/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <SuitableStruct/BufferReader.h>
#include <SuitableStruct/Exceptions.h>
#include <SuitableStruct/Hashes.h>

namespace SuitableStruct {

uint32_t BufferReader::hash() const
{
    return ssHashRaw(data(), rest());
}

void BufferReader::checkPosition(size_t pos) const
{
    if (pos > size())
        Internal::throwOutOfRange();
}

void BufferReader::checkAdvance(int64_t delta) const
{
    const auto newPos = static_cast<int64_t>(position()) + delta;

    if (newPos < 0 || newPos > size()) {
        Internal::throwOutOfRange();
    }
}

} // namespace SuitableStruct
