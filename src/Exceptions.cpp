#include <SuitableStruct/Internals/Exceptions.h>

#include <stdexcept>

namespace SuitableStruct {
namespace Internal {

void throwTooLarge()
{
    throw std::runtime_error("Can't load! Buffer is too large!");
}

void throwIntegrity()
{
    throw std::runtime_error("Integrity check failed!");
}

void throwOutOfRange()
{
    throw std::out_of_range("Out of range");
}

} // namespace Internal
} // namespace SuitableStruct
