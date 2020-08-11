#include <SuitableStruct/Hashes.h>

namespace SuitableStruct {

uint32_t ssHashRaw(const void* ptr, size_t sz)
{
    if (!sz)
        return 0;

    assert(ptr);

    uint32_t result = 0;
    size_t itemOffset = 0;
    while (sz >= 4) {
        result = combineHash(*((uint32_t*)ptr + itemOffset), result);
        itemOffset++;
        sz -= 4;
    }

    if (sz >= 2) {
        result = combineHash(*((uint16_t*)ptr), result);
        sz -= 2;
    }

    if (sz == 1) {
        result = combineHash(*((uint8_t*)ptr), result);
        sz -= 1;
    }

    assert(sz == 0);

    return result;
}

} // namespace SuitableStruct
