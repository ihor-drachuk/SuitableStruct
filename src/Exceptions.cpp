/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <SuitableStruct/Internals/Exceptions.h>

#include <stdexcept>

namespace SuitableStruct {
namespace Internal {

[[noreturn]] void throwTooLarge()
{
    throw std::runtime_error("Can't load! Buffer is too large!");
}

[[noreturn]] void throwIntegrity()
{
    throw std::runtime_error("Integrity check failed!");
}

[[noreturn]] void throwOutOfRange()
{
    throw std::out_of_range("Out of range");
}

} // namespace Internal
} // namespace SuitableStruct
