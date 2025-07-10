/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <SuitableStruct/Internals/Helpers.h>

#include <cassert>

namespace SuitableStruct {
namespace Internal {

// Private thread-local flags to track legacy format states
static thread_local std::optional<bool> OptIsProcessingLegacyBinFormat;
static thread_local std::optional<bool> OptIsProcessingLegacyJsonFormat;

std::optional<bool> isProcessingLegacyFormatOpt(FormatType formatType)
{
    const auto& optValue = [formatType]() -> const std::optional<bool>& {
        switch (formatType) {
            case FormatType::Binary: return OptIsProcessingLegacyBinFormat;
            case FormatType::Json:   return OptIsProcessingLegacyJsonFormat;
        }

        assert(false && "Should never reach here");
        return *reinterpret_cast<const std::optional<bool>*>(0);
    }();

    return optValue;
}

bool isProcessingLegacyFormat(FormatType formatType)
{
    const auto optValue = isProcessingLegacyFormatOpt(formatType);
    assert(optValue);
    return *optValue;
}

LegacyFormatScope::LegacyFormatScope(FormatType formatType, bool isLegacyFormat)
    : m_formatType(formatType),
      m_previousBinaryState(OptIsProcessingLegacyBinFormat),
      m_previousJsonState(OptIsProcessingLegacyJsonFormat)
{
    switch (formatType) {
        case FormatType::Binary:
            OptIsProcessingLegacyBinFormat = isLegacyFormat;
            break;
        case FormatType::Json:
            OptIsProcessingLegacyJsonFormat = isLegacyFormat;
            break;
    }
}

LegacyFormatScope::~LegacyFormatScope()
{
    switch (m_formatType) {
        case FormatType::Binary:
            OptIsProcessingLegacyBinFormat = m_previousBinaryState;
            break;
        case FormatType::Json:
            OptIsProcessingLegacyJsonFormat = m_previousJsonState;
            break;
    }
}

} // namespace Internal
} // namespace SuitableStruct
