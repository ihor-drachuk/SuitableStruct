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
} // namespace SuitableStruct
