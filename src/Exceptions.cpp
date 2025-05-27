/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <SuitableStruct/Exceptions.h>

namespace SuitableStruct {

VersionError::~VersionError() noexcept = default;
FormatError::~FormatError() noexcept = default;
IntegrityError::~IntegrityError() noexcept = default;

namespace Internal {

[[noreturn]] void throwTooLarge()
{
    throw std::length_error("Buffer is too large!");
}

[[noreturn]] void throwIntegrity()
{
    throw IntegrityError();
}

[[noreturn]] void throwOutOfRange()
{
    throw std::out_of_range("Out of range");
}

[[noreturn]] void throwVersionError()
{
    throw VersionError();
}

[[noreturn]] void throwFormat()
{
    throw FormatError();
}

} // namespace Internal
} // namespace SuitableStruct
