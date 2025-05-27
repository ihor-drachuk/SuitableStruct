/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

// Uncomment the line below to switch to GENERATE mode
//#define GENERATE_MODE

#include <gtest/gtest.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Internals/Exceptions.h>
#include <SuitableStruct/Containers/vector.h>
#include <vector>
#include <optional>
#include <chrono>
#include <variant>
#include <iostream>
#include <iomanip>
#include "utils/time_utils.h"

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QJsonValue>
#include <QJsonObject>
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

using namespace SuitableStruct;
using SuitableStructTest::toSeconds;

namespace {

constexpr auto ReferenceDurationRawSeconds = 1698408600;

#ifndef GENERATE_MODE
const uint8_t legacyFormatF0BufferData_v0[] = {
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x22, 0x91, 0x14, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x2a, 0x00, 0x00, 0x00, 0xc3, 0xf5, 0x48, 0x40, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x74, 0x65, 0x73, 0x74, 0x5f, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x5f, 0x76,
    0x30, 0x01, 0x00, 0x01, 0x7b, 0x00, 0x00, 0x00, 0x00, 0x6f, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x65, 0x73, 0x74, 0x65, 0x64, 0x5f, 0x76, 0x30
#else
    0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x22, 0x91, 0x14, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x2a, 0x00, 0x00, 0x00, 0xc3, 0xf5, 0x48, 0x40, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x74, 0x65, 0x73, 0x74, 0x5f, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x5f, 0x76,
    0x30, 0x01, 0x00, 0x01, 0x7b, 0x00, 0x00, 0x00, 0x00, 0x6f, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x65, 0x73, 0x74, 0x65, 0x64, 0x5f, 0x76, 0x30
#endif
};

const uint8_t legacyFormatF0BufferData_v1[] = {
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    0x9a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4f, 0x4f, 0x8d, 0x14, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x2a, 0x00, 0x00, 0x00, 0xc3, 0xf5, 0x48, 0x40, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x74, 0x65, 0x73, 0x74, 0x5f, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x5f, 0x76,
    0x31, 0x01, 0x00, 0x01, 0x7b, 0x00, 0x00, 0x00, 0x00, 0xde, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x65, 0x73, 0x74, 0x65, 0x64, 0x5f, 0x76, 0x31, 0x58,
    0x39, 0xb4, 0xc8, 0x76, 0xbe, 0x05, 0x40, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x00, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x1b, 0x7b, 0x22, 0x6b, 0x65, 0x79,
    0x31, 0x22, 0x3a, 0x22, 0x76, 0x61, 0x6c, 0x75, 0x65, 0x31, 0x22, 0x2c, 0x22, 0x6b, 0x65, 0x79,
    0x32, 0x22, 0x3a, 0x34, 0x32, 0x7d
#else
    0x75, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd5, 0x00, 0x93, 0x23, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x2a, 0x00, 0x00, 0x00, 0xc3, 0xf5, 0x48, 0x40, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x74, 0x65, 0x73, 0x74, 0x5f, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x5f, 0x76,
    0x31, 0x01, 0x00, 0x01, 0x7b, 0x00, 0x00, 0x00, 0x00, 0xde, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x65, 0x73, 0x74, 0x65, 0x64, 0x5f, 0x76, 0x31, 0x58,
    0x39, 0xb4, 0xc8, 0x76, 0xbe, 0x05, 0x40, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x05, 0x00, 0x00, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe7, 0x03, 0x00,
    0x00
#endif
};

const uint8_t legacyFormatF0BufferData_v2[] = {
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    0xe7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0xf7, 0xb9, 0x5e, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x2a, 0x00, 0x00, 0x00, 0xc3, 0xf5, 0x48, 0x40, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x74, 0x65, 0x73, 0x74, 0x5f, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x5f, 0x76,
    0x32, 0x01, 0x00, 0x01, 0x7b, 0x00, 0x00, 0x00, 0x00, 0x4d, 0x01, 0x00, 0x00, 0x00, 0x09, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x65, 0x73, 0x74, 0x65, 0x64, 0x5f, 0x76, 0x32, 0x33,
    0x33, 0x05, 0x42, 0x58, 0x39, 0xb4, 0xc8, 0x76, 0xbe, 0x05, 0x40, 0x00, 0x05, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x1b, 0x7b,
    0x22, 0x6b, 0x65, 0x79, 0x31, 0x22, 0x3a, 0x22, 0x76, 0x61, 0x6c, 0x75, 0x65, 0x31, 0x22, 0x2c,
    0x22, 0x6b, 0x65, 0x79, 0x32, 0x22, 0x3a, 0x34, 0x32, 0x7d, 0x00, 0x02, 0x00, 0x0c, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x61, 0x72, 0x69, 0x61, 0x6e, 0x74, 0x5f, 0x74, 0x65, 0x73,
    0x74, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x40, 0xe9, 0x38, 0x0c, 0xd5, 0x0b,
    0x81, 0x52, 0x00, 0xf4, 0xf1, 0x77, 0x86, 0x9f, 0xf5, 0x91, 0x17, 0x00, 0x01, 0x00, 0x0d, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6f, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x61, 0x6c, 0x5f, 0x74,
    0x65, 0x73, 0x74
#else
    0xc2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa5, 0x5b, 0xd1, 0x85, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x2a, 0x00, 0x00, 0x00, 0xc3, 0xf5, 0x48, 0x40, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x74, 0x65, 0x73, 0x74, 0x5f, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x5f, 0x76,
    0x32, 0x01, 0x00, 0x01, 0x7b, 0x00, 0x00, 0x00, 0x00, 0x4d, 0x01, 0x00, 0x00, 0x00, 0x09, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x65, 0x73, 0x74, 0x65, 0x64, 0x5f, 0x76, 0x32, 0x33,
    0x33, 0x05, 0x42, 0x58, 0x39, 0xb4, 0xc8, 0x76, 0xbe, 0x05, 0x40, 0x00, 0x05, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xe7, 0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x76, 0x61, 0x72, 0x69, 0x61, 0x6e, 0x74, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x00, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0x40, 0xe9, 0x38, 0x0c, 0xd5, 0x0b, 0x81, 0x52, 0x00, 0x2c, 0xf1,
    0x77, 0x86, 0x9f, 0xf5, 0x91, 0x17, 0x00, 0x01, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x6f, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x61, 0x6c, 0x5f, 0x74, 0x65, 0x73, 0x74
#endif
};
#endif // !GENERATE_MODE

#ifdef GENERATE_MODE
// Function to print buffer as C++ array
void printBufferAsArray(const Buffer& buffer, const std::string& name) {
    // Save current format flags
    std::ios_base::fmtflags savedFlags = std::cout.flags();
    char savedFill = std::cout.fill();

    std::cout << "const uint8_t " << name << "[] = {" << std::endl;
    std::cout << "    ";
    const uint8_t* data = buffer.data();
    for (size_t i = 0; i < buffer.size(); ++i) {
        if (i > 0 && i % 16 == 0) {
            std::cout << std::endl << "    ";
        }
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<unsigned>(data[i]);
        if (i < buffer.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << std::endl << "};" << std::endl << std::endl;

    // Restore original format flags
    std::cout.flags(savedFlags);
    std::cout.fill(savedFill);
}
#endif // GENERATE_MODE

struct NestedStruct_v2;

// Nested structure for v0 and v1
struct NestedStruct_v01
{
    int nestedInt {};
    std::string nestedString;

    NestedStruct_v01() = default;
    NestedStruct_v01(const NestedStruct_v01&) = default;
    NestedStruct_v01(NestedStruct_v01&&) = default;
    NestedStruct_v01& operator=(const NestedStruct_v01&) = default;
    NestedStruct_v01& operator=(NestedStruct_v01&&) = default;

    NestedStruct_v01(const NestedStruct_v2& v2);

#ifdef GENERATE_MODE
    void ssConvertFrom(const NestedStruct_v2& v2) {
        *this = NestedStruct_v01(v2);
    }
#endif // GENERATE_MODE

    auto ssTuple() const { return std::tie(nestedInt, nestedString); }
    SS_COMPARISONS_MEMBER(NestedStruct_v01)
};

// Nested structure for v2
struct NestedStruct_v2
{
    int nestedInt {};
    std::string nestedString;
    float nestedFloat {};

    NestedStruct_v2() = default;
    NestedStruct_v2(const NestedStruct_v2&) = default;
    NestedStruct_v2(NestedStruct_v2&&) = default;
    NestedStruct_v2& operator=(const NestedStruct_v2&) = default;
    NestedStruct_v2& operator=(NestedStruct_v2&&) = default;

    NestedStruct_v2(const NestedStruct_v01& v01)
        : nestedInt(v01.nestedInt)
        , nestedString(v01.nestedString)
        , nestedFloat(1.553f)
    {}

#ifdef GENERATE_MODE
    void ssConvertFrom(const NestedStruct_v01& v01) {
        *this = NestedStruct_v2(v01);
    }
#endif // GENERATE_MODE

    auto ssTuple() const { return std::tie(nestedInt, nestedString, nestedFloat); }
    SS_COMPARISONS_MEMBER(NestedStruct_v2)
};

NestedStruct_v01::NestedStruct_v01(const NestedStruct_v2 &v2)
    : nestedInt(v2.nestedInt)
    , nestedString(v2.nestedString)
{}

// Version 1: Basic structure with fundamental types
struct ComplexStruct_v0
{
    int intField {};
    float floatField {};
    std::string stringField;
    bool boolField {};
    std::optional<int> optionalField;
    NestedStruct_v01 nestedField;

    auto ssTuple() const { return std::tie(intField, floatField, stringField, boolField, optionalField, nestedField); }

    using ssVersions = std::tuple<ComplexStruct_v0>;
    SS_COMPARISONS_MEMBER_ONLY_EQ(ComplexStruct_v0)
};

// Version 2: Extended with more complex types
struct ComplexStruct_v1
{
    int intField {};
    float floatField {};
    std::string stringField;
    bool boolField {};
    std::optional<int> optionalField;
    NestedStruct_v01 nestedField;

    // New fields in v1
    double doubleField {};
    std::vector<int> vectorField;
    std::chrono::milliseconds durationField {};
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    QJsonValue jsonField;
#else
    int jsonField {};
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

    auto ssTuple() const {
        return std::tie(intField, floatField, stringField, boolField, optionalField, nestedField,
                       doubleField, vectorField, durationField, jsonField);
    }

    using ssVersions = std::tuple<ComplexStruct_v0, ComplexStruct_v1>;

    void ssUpgradeFrom(const ComplexStruct_v0& prev) {
        intField = prev.intField;
        floatField = prev.floatField;
        stringField = prev.stringField;
        boolField = prev.boolField;
        optionalField = prev.optionalField;
        nestedField = prev.nestedField;

        // Initialize new fields with non-zero defaults to detect upgrade
        doubleField = 99.99;
        vectorField = {100, 200};
        durationField = std::chrono::milliseconds(1000);
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
        jsonField = QJsonObject({{"upgraded", true}});
#else
        jsonField = 777;
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
    }

#ifdef GENERATE_MODE
    void ssConvertFrom(const ComplexStruct_v0& prev) {
        ssUpgradeFrom(prev);
    }
#endif // GENERATE_MODE

#ifndef GENERATE_MODE
    void ssDowngradeTo(ComplexStruct_v0& next) const {
        next.intField = intField;
        next.floatField = floatField;
        next.stringField = stringField;
        next.boolField = boolField;
        next.optionalField = optionalField;
        next.nestedField = nestedField;
    }
#endif // !GENERATE_MODE

    SS_COMPARISONS_MEMBER_ONLY_EQ(ComplexStruct_v1)
};

// Version 3: Even more complex with nested structures and variants
struct ComplexStruct_v2
{
    int intField {};
    float floatField {};
    std::string stringField;
    bool boolField {};
    std::optional<int> optionalField;
    NestedStruct_v2 nestedField; // Nested upgrade from v01

    // Fields from v1
    double doubleField {};
    std::vector<int> vectorField;
    std::chrono::milliseconds durationField {};
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    QJsonValue jsonField;
#else
    int jsonField {};
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

    // New fields in v2
    std::variant<std::monostate, int, std::string> variantField;
    std::chrono::steady_clock::time_point timePointField;
    std::optional<std::string> optionalStringField;

    auto ssTuple() const {
        return std::tie(intField, floatField, stringField, boolField, optionalField, nestedField,
                       doubleField, vectorField, durationField, jsonField,
                       variantField, timePointField, optionalStringField);
    }

    using ssVersions = std::tuple<ComplexStruct_v0, ComplexStruct_v1, ComplexStruct_v2>;

    void ssUpgradeFrom(const ComplexStruct_v1& prev) {
        intField = prev.intField;
        floatField = prev.floatField;
        stringField = prev.stringField;
        boolField = prev.boolField;
        optionalField = prev.optionalField;
        nestedField = NestedStruct_v2(prev.nestedField);
        doubleField = prev.doubleField;
        vectorField = prev.vectorField;
        durationField = prev.durationField;
        jsonField = prev.jsonField;

        // Initialize new fields with non-zero defaults to detect upgrade
        variantField = std::string("upgraded_v2");
        timePointField = SuitableStructTest::timeToSteadyClock(ReferenceDurationRawSeconds);
        optionalStringField = "upgraded_optional";
    }

#ifdef GENERATE_MODE
    void ssConvertFrom(const ComplexStruct_v1& prev) {
        ssUpgradeFrom(prev);
    }
#endif // GENERATE_MODE

#ifndef GENERATE_MODE
    void ssDowngradeTo(ComplexStruct_v1& next) const {
        next.intField = intField;
        next.floatField = floatField;
        next.stringField = stringField;
        next.boolField = boolField;
        next.optionalField = optionalField;
        next.nestedField = NestedStruct_v01(nestedField);
        next.doubleField = doubleField;
        next.vectorField = vectorField;
        next.durationField = durationField;
        next.jsonField = jsonField;
    }
#endif // !GENERATE_MODE

    SS_COMPARISONS_MEMBER_ONLY_EQ(ComplexStruct_v2)
};

bool compare_eq(const ComplexStruct_v2&,
                const std::chrono::steady_clock::time_point& a,
                const std::chrono::steady_clock::time_point& b)
{
    return std::chrono::abs(a - b) < std::chrono::seconds(1);
}

} // namespace

#ifdef GENERATE_MODE
// Test for serializing structures (to be used for generating legacy format (F0) buffers on old commit)
TEST(SuitableStruct, LegacyFormatF0CompatibilityTest_P_SerializeStructures)
{
    // Test data for v0
    ComplexStruct_v0 v0_value;
    v0_value.intField = 42;
    v0_value.floatField = 3.14f;
    v0_value.stringField = "test_string_v0";
    v0_value.boolField = true;
    v0_value.optionalField = 123;
    v0_value.nestedField.nestedInt = 111;
    v0_value.nestedField.nestedString = "nested_v0";

    // Test data for v1
    ComplexStruct_v1 v1_value;
    v1_value.intField = 42;
    v1_value.floatField = 3.14f;
    v1_value.stringField = "test_string_v1";
    v1_value.boolField = true;
    v1_value.optionalField = 123;
    v1_value.nestedField.nestedInt = 222;
    v1_value.nestedField.nestedString = "nested_v1";
    v1_value.doubleField = 2.718;
    v1_value.vectorField = {1, 2, 3, 4, 5};
    v1_value.durationField = std::chrono::milliseconds(500);
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    v1_value.jsonField = QJsonObject({{"key1", "value1"}, {"key2", 42}});
#else
    v1_value.jsonField = 999;
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

    // Test data for v2
    ComplexStruct_v2 v2_value;
    v2_value.intField = 42;
    v2_value.floatField = 3.14f;
    v2_value.stringField = "test_string_v2";
    v2_value.boolField = true;
    v2_value.optionalField = 123;
    v2_value.nestedField.nestedInt = 333;
    v2_value.nestedField.nestedString = "nested_v2";
    v2_value.doubleField = 2.718;
    v2_value.vectorField = {1, 2, 3, 4, 5};
    v2_value.durationField = std::chrono::milliseconds(500);
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    v2_value.jsonField = QJsonObject({{"key1", "value1"}, {"key2", 42}});
#else
    v2_value.jsonField = 999;
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
    v2_value.variantField = std::string("variant_test");
    v2_value.timePointField = SuitableStructTest::timeToSteadyClock(ReferenceDurationRawSeconds);
    v2_value.optionalStringField = "optional_test";
    v2_value.nestedField.nestedInt = 333;
    v2_value.nestedField.nestedString = "nested_v2";
    v2_value.nestedField.nestedFloat = 33.3f;

    // Serialize and verify they work in current format
    const auto v0_buffer = ssSave(v0_value);
    const auto v1_buffer = ssSave(v1_value);
    const auto v2_buffer = ssSave(v2_value);

    // GENERATE MODE: Print buffers for copying into legacy format (F0) buffer arrays
    std::cout << "=== GENERATE MODE: Buffer data for legacy format (F0) compatibility ===" << std::endl;
    std::cout << "v0 struct buffer size: " << v0_buffer.size() << " bytes" << std::endl;
    printBufferAsArray(v0_buffer, "legacyFormatF0BufferData_v0");

    std::cout << "v1 struct buffer size: " << v1_buffer.size() << " bytes" << std::endl;
    printBufferAsArray(v1_buffer, "legacyFormatF0BufferData_v1");

    std::cout << "v2 struct buffer size: " << v2_buffer.size() << " bytes" << std::endl;
    printBufferAsArray(v2_buffer, "legacyFormatF0BufferData_v2");

    // Test that we can load what we just generated
    ComplexStruct_v0 v0_loaded;
    ComplexStruct_v1 v1_loaded;
    ComplexStruct_v2 v2_loaded;
    ssLoad(v0_buffer, v0_loaded);
    ssLoad(v1_buffer, v1_loaded);
    ssLoad(v2_buffer, v2_loaded);

    ASSERT_EQ(v0_value, v0_loaded);
    ASSERT_EQ(v1_value, v1_loaded);
    ASSERT_EQ(v2_value, v2_loaded);
}
#endif // GENERATE_MODE

#ifndef GENERATE_MODE
// From legacy format F0, data v0 -> v0
TEST(SuitableStruct, LegacyFormatF0CompatibilityTest_P_LoadF0FormatBuffer_v0_to_v0)
{
    const Buffer legacyFormatF0Buffer(legacyFormatF0BufferData_v0, sizeof(legacyFormatF0BufferData_v0));
    ComplexStruct_v0 loaded;
    ssLoad(legacyFormatF0Buffer, loaded);

    // Verify the loaded data matches expected values
    ASSERT_EQ(loaded.intField, 42);
    ASSERT_FLOAT_EQ(loaded.floatField, 3.14f);
    ASSERT_EQ(loaded.stringField, "test_string_v0");
    ASSERT_TRUE(loaded.boolField);
    ASSERT_TRUE(loaded.optionalField.has_value());
    ASSERT_EQ(loaded.optionalField.value(), 123);
    ASSERT_EQ(loaded.nestedField.nestedInt, 111);
    ASSERT_EQ(loaded.nestedField.nestedString, "nested_v0");
}

// From legacy format F0, data v1 -> v0 (should fail - downgrade not supported)
TEST(SuitableStruct, LegacyFormatF0CompatibilityTest_P_LoadF0FormatBuffer_v1_to_v0_ShouldFail)
{
    const Buffer legacyFormatF0Buffer(legacyFormatF0BufferData_v1, sizeof(legacyFormatF0BufferData_v1));
    ComplexStruct_v0 loaded;

    // This should fail because v0 cannot load v1 data from legacy format (F0)
    EXPECT_THROW(ssLoad(legacyFormatF0Buffer, loaded), std::exception);
}

// From legacy format F0, data v0 -> v1 (upgrade)
TEST(SuitableStruct, LegacyFormatF0CompatibilityTest_P_LoadF0FormatBuffer_v0_to_v1)
{
    const Buffer legacyFormatF0Buffer(legacyFormatF0BufferData_v0, sizeof(legacyFormatF0BufferData_v0));
    ComplexStruct_v1 loaded;
    ssLoad(legacyFormatF0Buffer, loaded);

    // Verify the loaded data matches expected values from v0
    ASSERT_EQ(loaded.intField, 42);
    ASSERT_FLOAT_EQ(loaded.floatField, 3.14f);
    ASSERT_EQ(loaded.stringField, "test_string_v0");
    ASSERT_TRUE(loaded.boolField);
    ASSERT_TRUE(loaded.optionalField.has_value());
    ASSERT_EQ(loaded.optionalField.value(), 123);
    ASSERT_EQ(loaded.nestedField.nestedInt, 111);
    ASSERT_EQ(loaded.nestedField.nestedString, "nested_v0");

    // Verify v1-specific fields have upgrade values
    ASSERT_DOUBLE_EQ(loaded.doubleField, 99.99);
    ASSERT_EQ(loaded.vectorField, std::vector<int>({100, 200}));
    ASSERT_EQ(loaded.durationField, std::chrono::milliseconds(1000));
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    ASSERT_TRUE(loaded.jsonField.isObject());
#else
    ASSERT_EQ(loaded.jsonField, 777);
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
}

// From legacy format F0, data v1 -> v1
TEST(SuitableStruct, LegacyFormatF0CompatibilityTest_P_LoadF0FormatBuffer_v1_to_v1)
{
    const Buffer legacyFormatF0Buffer(legacyFormatF0BufferData_v1, sizeof(legacyFormatF0BufferData_v1));
    ComplexStruct_v1 loaded;
    ssLoad(legacyFormatF0Buffer, loaded);

    // Verify the loaded data matches expected values
    ASSERT_EQ(loaded.intField, 42);
    ASSERT_FLOAT_EQ(loaded.floatField, 3.14f);
    ASSERT_EQ(loaded.stringField, "test_string_v1");
    ASSERT_TRUE(loaded.boolField);
    ASSERT_TRUE(loaded.optionalField.has_value());
    ASSERT_EQ(loaded.optionalField.value(), 123);
    ASSERT_EQ(loaded.nestedField.nestedInt, 222);
    ASSERT_EQ(loaded.nestedField.nestedString, "nested_v1");
    ASSERT_DOUBLE_EQ(loaded.doubleField, 2.718);
    ASSERT_EQ(loaded.vectorField, std::vector<int>({1, 2, 3, 4, 5}));
    ASSERT_EQ(loaded.durationField, std::chrono::milliseconds(500));
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    ASSERT_TRUE(loaded.jsonField.isObject());
#else
    ASSERT_EQ(loaded.jsonField, 999);
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
}

// From legacy format F0, data v0 -> v2 (upgrade)
TEST(SuitableStruct, LegacyFormatF0CompatibilityTest_P_LoadF0FormatBuffer_v0_to_v2)
{
    const Buffer legacyFormatF0Buffer(legacyFormatF0BufferData_v0, sizeof(legacyFormatF0BufferData_v0));
    ComplexStruct_v2 loaded;
    ssLoad(legacyFormatF0Buffer, loaded);

    // Verify the loaded data matches expected values from v0
    ASSERT_EQ(loaded.intField, 42);
    ASSERT_FLOAT_EQ(loaded.floatField, 3.14f);
    ASSERT_EQ(loaded.stringField, "test_string_v0");
    ASSERT_TRUE(loaded.boolField);
    ASSERT_TRUE(loaded.optionalField.has_value());
    ASSERT_EQ(loaded.optionalField.value(), 123);
    ASSERT_EQ(loaded.nestedField.nestedInt, 111);
    ASSERT_EQ(loaded.nestedField.nestedString, "nested_v0");
    ASSERT_FLOAT_EQ(loaded.nestedField.nestedFloat, 1.553f);

    // Verify v1-specific fields have upgrade values (v0->v1 upgrade)
    ASSERT_DOUBLE_EQ(loaded.doubleField, 99.99);
    ASSERT_EQ(loaded.vectorField, std::vector<int>({100, 200}));
    ASSERT_EQ(loaded.durationField, std::chrono::milliseconds(1000));
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    ASSERT_TRUE(loaded.jsonField.isObject());
#else
    ASSERT_EQ(loaded.jsonField, 777);
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

    // Verify v2-specific fields have upgrade values (v1->v2 upgrade)
    ASSERT_TRUE(std::holds_alternative<std::string>(loaded.variantField));
    ASSERT_EQ(std::get<std::string>(loaded.variantField), "upgraded_v2");
    ASSERT_NEAR(toSeconds(loaded.timePointField), ReferenceDurationRawSeconds, 1.0);
    ASSERT_TRUE(loaded.optionalStringField.has_value());
    ASSERT_EQ(loaded.optionalStringField.value(), "upgraded_optional");
}

// From legacy format F0, data v1 -> v2 (upgrade)
TEST(SuitableStruct, LegacyFormatF0CompatibilityTest_P_LoadF0FormatBuffer_v1_to_v2)
{
    const Buffer legacyFormatF0Buffer(legacyFormatF0BufferData_v1, sizeof(legacyFormatF0BufferData_v1));
    ComplexStruct_v2 loaded;
    ssLoad(legacyFormatF0Buffer, loaded);

    // Verify the loaded data matches expected values from v1
    ASSERT_EQ(loaded.intField, 42);
    ASSERT_FLOAT_EQ(loaded.floatField, 3.14f);
    ASSERT_EQ(loaded.stringField, "test_string_v1");
    ASSERT_TRUE(loaded.boolField);
    ASSERT_TRUE(loaded.optionalField.has_value());
    ASSERT_EQ(loaded.optionalField.value(), 123);
    ASSERT_EQ(loaded.nestedField.nestedInt, 222);
    ASSERT_EQ(loaded.nestedField.nestedString, "nested_v1");
    ASSERT_FLOAT_EQ(loaded.nestedField.nestedFloat, 1.553f);
    ASSERT_DOUBLE_EQ(loaded.doubleField, 2.718);
    ASSERT_EQ(loaded.vectorField, std::vector<int>({1, 2, 3, 4, 5}));
    ASSERT_EQ(loaded.durationField, std::chrono::milliseconds(500));
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    ASSERT_TRUE(loaded.jsonField.isObject());
#else
    ASSERT_EQ(loaded.jsonField, 999);
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

    // Verify v2-specific fields have upgrade values (v1->v2 upgrade)
    ASSERT_TRUE(std::holds_alternative<std::string>(loaded.variantField));
    ASSERT_EQ(std::get<std::string>(loaded.variantField), "upgraded_v2");
    ASSERT_NEAR(toSeconds(loaded.timePointField), ReferenceDurationRawSeconds, 1.0);
    ASSERT_TRUE(loaded.optionalStringField.has_value());
    ASSERT_EQ(loaded.optionalStringField.value(), "upgraded_optional");
}

// From legacy format F0, data v2 -> v2
TEST(SuitableStruct, LegacyFormatF0CompatibilityTest_P_LoadF0FormatBuffer_v2_to_v2)
{
    const Buffer legacyFormatF0Buffer(legacyFormatF0BufferData_v2, sizeof(legacyFormatF0BufferData_v2));
    ComplexStruct_v2 loaded;
    ssLoad(legacyFormatF0Buffer, loaded);

    // Verify the loaded data matches expected values
    ASSERT_EQ(loaded.intField, 42);
    ASSERT_FLOAT_EQ(loaded.floatField, 3.14f);
    ASSERT_EQ(loaded.stringField, "test_string_v2");
    ASSERT_TRUE(loaded.boolField);
    ASSERT_TRUE(loaded.optionalField.has_value());
    ASSERT_EQ(loaded.optionalField.value(), 123);
    ASSERT_EQ(loaded.nestedField.nestedInt, 333);
    ASSERT_EQ(loaded.nestedField.nestedString, "nested_v2");
    ASSERT_FLOAT_EQ(loaded.nestedField.nestedFloat, 33.3f);
    ASSERT_DOUBLE_EQ(loaded.doubleField, 2.718);
    ASSERT_EQ(loaded.vectorField, std::vector<int>({1, 2, 3, 4, 5}));
    ASSERT_EQ(loaded.durationField, std::chrono::milliseconds(500));
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    ASSERT_TRUE(loaded.jsonField.isObject());
#else
    ASSERT_EQ(loaded.jsonField, 999);
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
    ASSERT_TRUE(std::holds_alternative<std::string>(loaded.variantField));
    ASSERT_EQ(std::get<std::string>(loaded.variantField), "variant_test");
    ASSERT_NEAR(toSeconds(loaded.timePointField), ReferenceDurationRawSeconds, 1.0);
    ASSERT_TRUE(loaded.optionalStringField.has_value());
    ASSERT_EQ(loaded.optionalStringField.value(), "optional_test");
}

// From legacy format F0, data v2 -> v0 (should fail - downgrade not supported)
TEST(SuitableStruct, LegacyFormatF0CompatibilityTest_P_LoadF0FormatBuffer_v2_to_v0_ShouldFail)
{
    const Buffer legacyFormatF0Buffer(legacyFormatF0BufferData_v2, sizeof(legacyFormatF0BufferData_v2));
    ComplexStruct_v0 loaded;

    // This should fail because v0 cannot load v2 data from legacy format (F0)
    EXPECT_THROW(ssLoad(legacyFormatF0Buffer, loaded), std::exception);
}

// From legacy format F0, data v2 -> v1 (should fail - downgrade not supported)
TEST(SuitableStruct, LegacyFormatF0CompatibilityTest_P_LoadF0FormatBuffer_v2_to_v1_ShouldFail)
{
    const Buffer legacyFormatF0Buffer(legacyFormatF0BufferData_v2, sizeof(legacyFormatF0BufferData_v2));
    ComplexStruct_v1 loaded;

    // This should fail because v1 cannot load v2 data from legacy format (F0)
    EXPECT_THROW(ssLoad(legacyFormatF0Buffer, loaded), std::exception);
}

// Test current format compatibility - ensure new format works correctly
TEST(SuitableStruct, LegacyFormatF0CompatibilityTest_P_CurrentFormat)
{
    ComplexStruct_v2 original;

    // Fill with comprehensive test data
    original.intField = 42;
    original.floatField = 3.14f;
    original.stringField = "test_string";
    original.boolField = true;
    original.optionalField = 123;
    original.nestedField.nestedInt = 555;
    original.nestedField.nestedString = "nested_current";
    original.nestedField.nestedFloat = 55.5f;
    original.doubleField = 2.718;
    original.vectorField = {1, 2, 3, 4, 5};
    original.durationField = std::chrono::milliseconds(500);
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    original.jsonField = QJsonObject({{"key1", "value1"}, {"key2", 42}});
#else
    original.jsonField = 999;
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
    original.variantField = std::string("variant_test");
    original.timePointField = SuitableStructTest::timeToSteadyClock(ReferenceDurationRawSeconds);
    original.optionalStringField = "optional_test";

    // Test serialization and deserialization with new F1 format
    const auto buffer = ssSave(original);
    ComplexStruct_v2 loaded;
    ssLoad(buffer, loaded);

    ASSERT_EQ(original, loaded);
}

// Test version downgrade compatibility
TEST(SuitableStruct, LegacyFormatF0CompatibilityTest_P_VersionDowngrade)
{
    ComplexStruct_v2 v2;
    v2.intField = 42;
    v2.floatField = 3.14f;
    v2.stringField = "test";
    v2.boolField = true;
    v2.optionalField = 123;
    v2.nestedField.nestedInt = 777;
    v2.nestedField.nestedString = "nested_downgrade";
    v2.nestedField.nestedFloat = 77.7f;
    v2.doubleField = 2.718;
    v2.vectorField = {1, 2, 3};
    v2.durationField = std::chrono::milliseconds(500);
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    v2.jsonField = QJsonValue("test");
#else
    v2.jsonField = 999;
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
    v2.variantField = std::string("test_variant");
    v2.timePointField = SuitableStructTest::timeToSteadyClock(ReferenceDurationRawSeconds);
    v2.optionalStringField = "test_optional";

    // Save v2 and load as v1
    const auto buffer = ssSave(v2);
    ComplexStruct_v1 v1;
    ssLoad(buffer, v1);

    // Verify common fields are preserved
    ASSERT_EQ(v1.intField, v2.intField);
    ASSERT_EQ(v1.floatField, v2.floatField);
    ASSERT_EQ(v1.stringField, v2.stringField);
    ASSERT_EQ(v1.boolField, v2.boolField);
    ASSERT_EQ(v1.optionalField, v2.optionalField);
    ASSERT_EQ(v1.nestedField.nestedInt, v2.nestedField.nestedInt);
    ASSERT_EQ(v1.nestedField.nestedString, v2.nestedField.nestedString);
    ASSERT_EQ(v1.doubleField, v2.doubleField);
    ASSERT_EQ(v1.vectorField, v2.vectorField);
    ASSERT_EQ(v1.durationField, v2.durationField);
    ASSERT_EQ(v1.jsonField, v2.jsonField);

    // Save v1 and load as v0
    const auto buffer2 = ssSave(v1);
    ComplexStruct_v0 v0;
    ssLoad(buffer2, v0);

    // Verify common fields are preserved
    ASSERT_EQ(v0.intField, v1.intField);
    ASSERT_EQ(v0.floatField, v1.floatField);
    ASSERT_EQ(v0.stringField, v1.stringField);
    ASSERT_EQ(v0.boolField, v1.boolField);
    ASSERT_EQ(v0.optionalField, v1.optionalField);
    ASSERT_EQ(v0.nestedField, v1.nestedField);
}
#endif // !GENERATE_MODE
