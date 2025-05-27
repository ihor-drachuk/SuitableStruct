/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <SuitableStruct/Internals/Helpers.h>

namespace SuitableStruct {
namespace Internal {

// Private thread-local flags to track legacy format states
static thread_local bool IsProcessingLegacyBinFormat = false;
static thread_local bool IsProcessingLegacyJsonFormat = false;

bool isProcessingLegacyFormat(FormatType formatType)
{
    switch (formatType) {
        case FormatType::Binary:
            return IsProcessingLegacyBinFormat;
        case FormatType::Json:
            return IsProcessingLegacyJsonFormat;
    }
    return false; // Should never reach here
}

LegacyFormatScope::LegacyFormatScope(FormatType formatType, bool isLegacyFormat)
    : m_formatType(formatType),
      m_previousBinaryState(IsProcessingLegacyBinFormat),
      m_previousJsonState(IsProcessingLegacyJsonFormat)
{
    switch (formatType) {
        case FormatType::Binary:
            IsProcessingLegacyBinFormat = isLegacyFormat;
            break;
        case FormatType::Json:
            IsProcessingLegacyJsonFormat = isLegacyFormat;
            break;
    }
}

LegacyFormatScope::~LegacyFormatScope()
{
    switch (m_formatType) {
        case FormatType::Binary:
            IsProcessingLegacyBinFormat = m_previousBinaryState;
            break;
        case FormatType::Json:
            IsProcessingLegacyJsonFormat = m_previousJsonState;
            break;
    }
}

} // namespace Internal
} // namespace SuitableStruct
