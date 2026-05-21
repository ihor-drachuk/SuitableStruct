/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <SuitableStruct/Serializer.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QByteArray>
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

namespace SuitableStruct {
namespace Internal {

// Format constants
const uint8_t SS_FORMAT_F0[SS_FORMAT_MARK_SIZE] = { 0, 0, 0, 0, 0 };  // Format F0, single-version, old hash algorithm
const uint8_t SS_FORMAT_F1[SS_FORMAT_MARK_SIZE] = { 1, 0, 0, 0, 0 };  // Format F1, multiple versions segments, new hash algorithm

} // namespace Internal

namespace {

struct ValidatedPayload {
    BufferReader payload;
    SSDataFormat format {SSDataFormat::F1};
};

struct PositionRestorer {
    BufferReader& reader;
    size_t position {};

    PositionRestorer(BufferReader& r, size_t p) : reader(r), position(p) {}
    ~PositionRestorer() { reader.seek(position); }

    PositionRestorer(const PositionRestorer&) = delete;
    PositionRestorer(PositionRestorer&&) = delete;
    PositionRestorer& operator=(const PositionRestorer&) = delete;
    PositionRestorer& operator=(PositionRestorer&&) = delete;
};

// Does NOT restore the input bufferReader position — caller's responsibility.
std::optional<ValidatedPayload> readValidatedPayload(BufferReader& bufferReader)
{
    try {
        const auto size = bufferReader.read<uint64_t>();
        const auto hash = bufferReader.read<uint32_t>();

        BufferReader payloadReader = bufferReader.readRaw(size);

        const auto computedHashF1 = payloadReader.hash();
        const auto computedHashF0 = ssHashRaw_F0(payloadReader.data(), payloadReader.size());

        const bool hashValid = (hash == computedHashF1) || (hash == computedHashF0);
        if (!hashValid)
            return {};

        uint8_t formatMarker[Internal::SS_FORMAT_MARK_SIZE];
        payloadReader.readRaw(formatMarker, sizeof(formatMarker));

        const bool isF0 = memcmp(formatMarker, Internal::SS_FORMAT_F0, Internal::SS_FORMAT_MARK_SIZE) == 0;
        const bool isF1 = !isF0 && memcmp(formatMarker, Internal::SS_FORMAT_F1, Internal::SS_FORMAT_MARK_SIZE) == 0;

        if (isF0) {
            return ValidatedPayload{payloadReader, SSDataFormat::F0};
        } else if (isF1) {
            return ValidatedPayload{payloadReader, SSDataFormat::F1};
        } else {
            return {};
        }

    } catch (...) {
        return {};
    }
}

} // anonymous namespace

std::optional<SSDataFormat> ssDetectFormat(const Buffer& buffer)
{
    BufferReader reader(buffer);
    return ssDetectFormat(reader);
}

std::optional<SSDataFormat> ssDetectFormat(BufferReader& bufferReader)
{
    // Save current position to restore later
    PositionRestorer restorer{bufferReader, bufferReader.position()};

    const auto optResult = readValidatedPayload(bufferReader);
    if (!optResult)
        return {};
    return optResult->format;
}

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
std::optional<SSDataFormat> ssDetectFormat(const QByteArray& buffer)
{
    return ssDetectFormat(Buffer(buffer));
}
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

std::optional<uint8_t> ssPeekVersion(const Buffer& buffer)
{
    BufferReader reader(buffer);
    return ssPeekVersion(reader);
}

std::optional<uint8_t> ssPeekVersion(BufferReader& bufferReader)
{
    PositionRestorer restorer{bufferReader, bufferReader.position()};

    try {
        auto optResult = readValidatedPayload(bufferReader);
        if (!optResult)
            return {};
        if (optResult->format == SSDataFormat::F0)
            return {};

        BufferReader& payload = optResult->payload;
        uint8_t segmentsCount {};
        payload.read(segmentsCount);
        if (segmentsCount == 0)
            return {};

        std::optional<uint8_t> optMaxVersion;
        for (uint8_t i = 0; i < segmentsCount; ++i) {
            uint8_t storedVersion {};
            payload.read(storedVersion);
            const auto segmentSize = payload.read<uint64_t>();
            if (segmentSize > payload.rest())
                return {};
            payload.seek(payload.position() + static_cast<size_t>(segmentSize));
            optMaxVersion = std::max(optMaxVersion.value_or(0), storedVersion);
        }
        return optMaxVersion;

    } catch (...) {
        return {};
    }
}

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
std::optional<uint8_t> ssPeekVersion(const QByteArray& buffer)
{
    return ssPeekVersion(Buffer(buffer));
}
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

} // namespace SuitableStruct
