/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <SuitableStruct/BufferReader.h>
#include <SuitableStruct/Exceptions.h>
#include <SuitableStruct/Hashes.h>
#include <limits>

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

void BufferReader::checkAdvance(size_t delta) const
{
    // Check for overflow when adding delta to current position
    if (delta > size() - position())
        Internal::throwOutOfRange();
}

void BufferReader::checkAdvance(std::ptrdiff_t delta) const
{
    if (!delta)
        return;

    const auto currentPos = position();
    const auto bufferSize = size();

    if (delta > 0) {
        const auto newPosition = currentPos + static_cast<size_t>(delta);

        if (newPosition > bufferSize)
            Internal::throwOutOfRange();
    } else {
        if (delta == std::numeric_limits<std::ptrdiff_t>::min())
            Internal::throwOutOfRange();

        const auto unsignedAbsDelta = static_cast<size_t>(-delta);

        if (currentPos < unsignedAbsDelta)
            Internal::throwOutOfRange();
    }
}

} // namespace SuitableStruct
