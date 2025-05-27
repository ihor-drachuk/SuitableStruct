/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>
#include <vector>
#include <optional>
#include <chrono>
#include <iostream>
#include <iomanip>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QJsonValue>
#include <QJsonObject>
#include <QDateTime>
#include <QTimeZone>
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

using namespace SuitableStruct;

namespace {

struct SomeStruct1
{
    int a {};
    std::string b;

    auto ssTuple() const { return std::tie(a, b); }
};

SS_COMPARISONS(SomeStruct1)

struct SomeStruct2
{
    SomeStruct1 struct1;
    float c {};
    std::optional<int> d;
    bool e {};
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    QJsonValue f;
#else
    int f {};
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
    std::chrono::steady_clock::time_point g1;
    std::chrono::system_clock::time_point g2;
    std::chrono::hours h {};
    std::chrono::milliseconds i {};
    std::variant<std::monostate, int, std::string> j;

    auto ssTuple() const { return std::tie(struct1, c, d, e, f, g1, g2, h, i, j); }
};

SS_COMPARISONS_ONLY_EQ(SomeStruct2)

inline bool compare_eq(const SomeStruct2&, const std::chrono::steady_clock::time_point& a, const std::chrono::steady_clock::time_point& b)
{
    const auto diff = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(a - b).count());
    return diff <= 1;
}

} // namespace


TEST(SuitableStruct, SerializationTest)
{
    SomeStruct2 value1;

    value1.struct1.a = 1;
    value1.struct1.b = "sdfsdf";
    value1.c = 2.5;
    value1.d = 150;
    value1.e = true;
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    value1.f = QJsonObject({{"SubValue1", 1}, {"SubValue2", "b"}});
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
    value1.g1 = std::chrono::steady_clock::now();
    value1.g2 = std::chrono::system_clock::now() + std::chrono::minutes(10);
    value1.h = std::chrono::hours(5);
    value1.i = std::chrono::milliseconds(123);
    value1.j = "sdfg";

    SomeStruct2 value2;
    ASSERT_NE(value1, value2);

    auto saved = ssSave(value1);
    BufferReader reader(saved);
    ssLoad(reader, value2);
    ASSERT_EQ(reader.rest(), 0);

    ASSERT_EQ(value1, value2);

    value1.d.reset();

    saved = ssSave(value1);
    reader.resetPosition();
    ssLoad(reader, value2);

    ASSERT_EQ(value1, value2);
}

TEST(SuitableStruct, SerializationTest_Corruption)
{
    SomeStruct2 initialStruct;
    initialStruct.struct1.a = 1;
    initialStruct.struct1.b = "sdfsdf";
    initialStruct.c = 2.5;

    const auto savedBuffer = ssSave(initialStruct);
    const auto len = savedBuffer.size();

    for (size_t i = 0; i < len; i++) {
        SomeStruct2 value2;
        ASSERT_NE(initialStruct, value2);

        auto savedCopy = savedBuffer;
        savedCopy.data()[i]++;
        ASSERT_THROW(ssLoad(savedCopy, value2), std::exception);
    }

    for (size_t i = 1; i < len; i++) {
        SomeStruct2 value2;
        ASSERT_NE(initialStruct, value2);

        auto savedCopy = savedBuffer;
        savedCopy.reduceSize(i);
        ASSERT_THROW(ssLoad(savedCopy, value2), std::exception);
    }
}

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
TEST(SuitableStruct, SerializationTest_DateTime)
{
    const QDate date(2023, 05, 05);
    const QTime time(1, 54, 12, 127);

    const std::vector<QDateTime> testData {
        QDateTime::currentDateTime(),
        QDateTime::currentDateTimeUtc(),
        QDateTime(date, time, Qt::LocalTime),
        QDateTime(date, time, Qt::UTC),
        QDateTime(date, time, Qt::OffsetFromUTC, 60*60*2),
        QDateTime(date, time, QTimeZone("America/New_York")),
        QDateTime(date, time, QTimeZone("sdfgihdsfg"))
    };

    for (const auto& x : testData) {
        const auto buffer = ssSave(x, false);
        const auto readBack = ssLoadRet<QDateTime>(buffer, false);
        ASSERT_EQ(x, readBack);
        ASSERT_EQ(x.offsetFromUtc(), readBack.offsetFromUtc());
        ASSERT_EQ(x.timeZone(), readBack.timeZone());
    }
}
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

TEST(SuitableStruct, SerializationTest_OldBoolCompatibility)
{
    const Buffer oldTrue("\1", 1);
    const Buffer oldFalse("\0", 1);
    BufferReader oldTrueReader(oldTrue);
    BufferReader oldFalseReader(oldFalse);
    const auto readTrue = ssLoadImplRet<bool>(BufferReader(oldTrue));
    const auto readFalse = ssLoadImplRet<bool>(BufferReader(oldFalse));
    ASSERT_TRUE(readTrue);
    ASSERT_FALSE(readFalse);
}

//#define GENERATE_MODE
TEST(SuitableStruct, SerializationTest_PreSerializedSteadyClock)
{
    // Fixed reference time
    constexpr auto referenceEpochMs = 1704110400000LL; // 2024-01-01 12:00:00 UTC in milliseconds
    const auto referenceSystemTime = std::chrono::system_clock::time_point(std::chrono::milliseconds(referenceEpochMs));

    // Generation mode - create test data for the reference time
    const auto genCurrentSystemTime = std::chrono::system_clock::now();
    const auto genCurrentSteadyTime = std::chrono::steady_clock::now();

    // Calculate what steady_clock time would represent our reference time
    const auto diffToReference = referenceSystemTime - genCurrentSystemTime;
    const auto referenceSteadyTime = genCurrentSteadyTime + diffToReference;

    const auto serialized = ssSave(referenceSteadyTime);

#ifdef GENERATE_MODE
    std::cout << "\n=== Pre-serialized steady_clock ===\n";
    std::cout << "Reference epoch ms: " << referenceEpochMs << "\n";
    std::cout << "Buffer size: " << serialized.size() << " bytes\n";
    std::cout << "Hex data: ";
    for (size_t i = 0; i < serialized.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(static_cast<unsigned char>(serialized.data()[i]));
    }
    std::cout << std::dec << "\n";
    std::cout << "C++ literal: Buffer(\"";
    for (size_t i = 0; i < serialized.size(); ++i) {
        std::cout << "\\x" << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(static_cast<unsigned char>(serialized.data()[i]));
    }
    std::cout << "\", " << std::dec << serialized.size() << ");\n";
    std::cout << "=====================================\n" << std::endl;

    // In generation mode, verify round-trip works
    BufferReader reader(serialized);
    const auto loadedTime = ssLoadRet<std::chrono::steady_clock::time_point>(reader);
    const auto loadedDiff = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(loadedTime - referenceSteadyTime).count());
    ASSERT_LE(loadedDiff, 1);  // Should be nearly identical
#else
    // Testing mode - use pre-serialized data

    const Buffer preSerializedData("\x1f\x00\x00\x00\x00\x00\x00\x00\x5c\xe9\x23\xfc\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x40\xe9\x38\x0c\xd5\x0b\x81\x52\x00\x2c\x81\x0c\x4a\x61\x37\xa6\x17", 43);
    ASSERT_NE(preSerializedData.size(), serialized.size());

    BufferReader reader(preSerializedData);
    const auto loadedTime = ssLoadRet<std::chrono::steady_clock::time_point>(reader);

    // Convert loaded steady_clock time back to system_clock to verify
    const auto currentSystemTime = std::chrono::system_clock::now();
    const auto currentSteadyTime = std::chrono::steady_clock::now();
    const auto loadedSystemTime = currentSystemTime + (loadedTime - currentSteadyTime);

    // The loaded time should represent our reference time
    const auto diffMs = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(loadedSystemTime - referenceSystemTime).count());
    ASSERT_LE(diffMs, 1000);  // Allow 1 second tolerance for clock drift and test execution time
#endif // GENERATE_MODE
}
