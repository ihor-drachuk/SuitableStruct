/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <SuitableStruct/Hashes.h>

#include <cassert>
#include <cstddef>

namespace SuitableStruct {

uint32_t ssHashRaw_F0(const void* ptr, size_t sz) // Legacy format F0
{
    if (!sz)
        return 0;

    assert(ptr);

    uint32_t result = 0;
    size_t itemOffset = 0;
    while (sz >= 4) {
        result = Internal::combineHash(*((uint32_t*)ptr + itemOffset), result);
        itemOffset++;
        sz -= 4;
    }

    if (sz >= 2) {
        result = Internal::combineHash(*((uint16_t*)ptr), result); // Error! Offset is not taken into account!
        sz -= 2;
    }

    if (sz == 1) {
        result = Internal::combineHash(*((uint8_t*)ptr), result); // Error! Offset is not taken into account!
        sz -= 1;
    }

    assert(sz == 0);

    return result;
}

// New FNV-1a 32-bit hash implementation (F1)
uint32_t ssHashRaw_F1(const void* ptr, size_t sz)
{
    constexpr uint32_t fnvOffsetBasis = 2166136261u;
    constexpr uint32_t fnvPrime = 16777619u;

    const auto* data = static_cast<const uint8_t*>(ptr);
    uint32_t hash = fnvOffsetBasis;

    for (size_t i = 0; i < sz; ++i) {
        hash ^= data[i];
        hash *= fnvPrime;
    }

    return hash;
}

uint32_t ssHashRaw(const void *ptr, size_t sz)
{
    return ssHashRaw_F1(ptr, sz);
}

} // namespace SuitableStruct
