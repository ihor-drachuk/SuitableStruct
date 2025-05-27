/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once

#include <chrono>

namespace SuitableStructTest {

inline std::chrono::steady_clock::time_point systemClockToSteadyClock(const std::chrono::system_clock::time_point& systemTp)
{
    const auto diffFromSystemNow = systemTp - std::chrono::system_clock::now();
    const auto result = std::chrono::steady_clock::now() + diffFromSystemNow;
    return std::chrono::time_point_cast<std::chrono::steady_clock::duration>(result);
}

inline std::chrono::steady_clock::time_point timeToSteadyClock(std::time_t timestamp)
{
    const auto systemTp = std::chrono::system_clock::from_time_t(timestamp);
    return systemClockToSteadyClock(systemTp);
}

inline std::chrono::system_clock::time_point steadyClockToSystemClock(const std::chrono::steady_clock::time_point& steadyTp)
{
    const auto diffFromSteadyNow = steadyTp - std::chrono::steady_clock::now();
    const auto result = std::chrono::system_clock::now() + diffFromSteadyNow;
    return std::chrono::time_point_cast<std::chrono::system_clock::duration>(result);
}

inline double toSeconds(const std::chrono::steady_clock::time_point& tp)
{
    const auto systemTp = steadyClockToSystemClock(tp);
    using SecondsDbl = std::chrono::duration<double, std::ratio<1>>;
    const auto duration = std::chrono::duration_cast<SecondsDbl>(systemTp.time_since_epoch());
    return duration.count();
}

} // namespace SuitableStructTest
