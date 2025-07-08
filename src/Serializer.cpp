/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <SuitableStruct/Serializer.h>

namespace SuitableStruct {
namespace Internal {

// Format constants
const uint8_t SS_FORMAT_F0[SS_FORMAT_MARK_SIZE] = { 0, 0, 0, 0, 0 };  // Format F0, single-version, old hash algorithm
const uint8_t SS_FORMAT_F1[SS_FORMAT_MARK_SIZE] = { 1, 0, 0, 0, 0 };  // Format F1, multiple versions segments, new hash algorithm

} // namespace Internal

std::optional<SSDataFormat> ssDetectFormat(const Buffer& buffer)
{
    BufferReader reader(buffer);
    return ssDetectFormat(reader);
}

std::optional<SSDataFormat> ssDetectFormat(BufferReader& bufferReader)
{
    // Save current position to restore later
    const size_t originalPosition = bufferReader.position();

    const auto deleter = [&](void*) { bufferReader.seek(originalPosition); };
    const auto positionRestorer = std::unique_ptr<void, decltype(deleter)>((void*)(1), deleter);

    try {
        const auto size = bufferReader.read<uint64_t>();
        const auto hash = bufferReader.read<uint32_t>();

        BufferReader payloadReader = bufferReader.readRaw(size);

        const auto computedHashF1 = payloadReader.hash();
        const auto computedHashF0 = ssHashRaw_F0(payloadReader.data(), payloadReader.size());

        bool hashValid = (hash == computedHashF1) || (hash == computedHashF0);
        if (!hashValid)
            return {};

        uint8_t formatMarker[Internal::SS_FORMAT_MARK_SIZE];
        payloadReader.readRaw(formatMarker, sizeof(formatMarker));

        const bool isF0 = memcmp(formatMarker, Internal::SS_FORMAT_F0, Internal::SS_FORMAT_MARK_SIZE) == 0;
        const bool isF1 = !isF0 && memcmp(formatMarker, Internal::SS_FORMAT_F1, Internal::SS_FORMAT_MARK_SIZE) == 0;

        if (isF0) {
            return SSDataFormat::F0;
        } else if (isF1) {
            return SSDataFormat::F1;
        } else {
            return {};
        }

    } catch (...) {
        return {};
    }
}

} // namespace SuitableStruct
