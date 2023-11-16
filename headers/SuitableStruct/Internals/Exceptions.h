/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once

namespace SuitableStruct {
namespace Internal {

[[noreturn]] void throwTooLarge();
[[noreturn]] void throwIntegrity();
[[noreturn]] void throwOutOfRange();

} // namespace Internal
} // namespace SuitableStruct
