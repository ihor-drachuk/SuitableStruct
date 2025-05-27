/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <stdexcept>

namespace SuitableStruct {

class VersionError : public std::runtime_error
{
public:
    VersionError() : std::runtime_error("Version error") {}
    VersionError(const VersionError&) = default;
    explicit VersionError(const char* message) : std::runtime_error(message) {}
    ~VersionError() noexcept override;
};

class FormatError : public std::runtime_error
{
public:
    FormatError() : std::runtime_error("Format error") {}
    FormatError(const FormatError&) = default;
    explicit FormatError(const char* message) : std::runtime_error(message) {}
    ~FormatError() noexcept override;
};

class IntegrityError : public std::runtime_error
{
public:
    IntegrityError() : std::runtime_error("Integrity error") {}
    IntegrityError(const IntegrityError&) = default;
    explicit IntegrityError(const char* message) : std::runtime_error(message) {}
    ~IntegrityError() noexcept override;
};

namespace Internal {

[[noreturn]] void throwTooLarge();
[[noreturn]] void throwIntegrity();
[[noreturn]] void throwOutOfRange();
[[noreturn]] void throwVersionError();
[[noreturn]] void throwFormat();

} // namespace Internal
} // namespace SuitableStruct
