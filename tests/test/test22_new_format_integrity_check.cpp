/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

// Replace by "1" to enable test/generator
#if 0

// Uncomment to switch to GENERATE mode
#define GENERATE_MODE

#include <gtest/gtest.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Containers/vector.h>
#include <vector>
#include <optional>
#include <iostream>
#include <iomanip>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QJsonValue>
#include <QJsonObject>
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

using namespace SuitableStruct;

namespace {

#ifndef GENERATE_MODE
// These buffers contain data serialized with new format (F1) that should cause
// integrity exceptions when loaded with old implementation due to hash function changes
const uint8_t newFormatF1BufferData_simple[] = {
    0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xa5, 0xd3, 0x71, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x6e, 0x65, 0x77, 0x5f, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x5f, 0x74, 0x65, 0x73, 0x74,
    0x01
};

const uint8_t newFormatF1BufferData_complex[] = {
    0xb3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xa1, 0xdb, 0x37, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x01, 0x6a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7b, 0x00, 0x00, 0x00, 0xd0,
    0x0f, 0x49, 0x40, 0x01, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x78, 0x5f, 0x6e, 0x65, 0x77,
    0x5f, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x01, 0x00, 0x1c, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00,
    0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x32, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x90, 0xf7, 0xaa,
    0x95, 0x09, 0xbf, 0x05, 0x40, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7b, 0x00,
    0x00, 0x00, 0xd0, 0x0f, 0x49, 0x40, 0x01, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x78, 0x5f,
    0x6e, 0x65, 0x77, 0x5f, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x5f, 0x74, 0x65, 0x73, 0x74
};
#endif // !GENERATE_MODE

#ifdef GENERATE_MODE
void printBufferAsArray(const Buffer& buffer, const std::string& name) {
    // Save current format flags
    const std::ios_base::fmtflags savedFlags = std::cout.flags();
    const char savedFill = std::cout.fill();

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

struct SimpleTestStruct
{
    int value {};
    std::string text;
    bool flag {};

    auto ssTuple() const { return std::tie(value, text, flag); }
    SS_COMPARISONS_MEMBER(SimpleTestStruct)
};

struct ComplexTestStruct_v0
{
    int intField {};
    float floatField {};
    std::string stringField;

    using ssVersions = std::tuple<ComplexTestStruct_v0>;
    auto ssTuple() const { return std::tie(intField, floatField, stringField); }
    SS_COMPARISONS_MEMBER(ComplexTestStruct_v0)
};

struct ComplexTestStruct_v1
{
    int intField {};
    float floatField {};
    std::string stringField;
    std::vector<int> vectorField;
    std::optional<double> optionalField;

    using ssVersions = std::tuple<ComplexTestStruct_v0, ComplexTestStruct_v1>;
    auto ssTuple() const { return std::tie(intField, floatField, stringField, vectorField, optionalField); }

    void ssUpgradeFrom(const ComplexTestStruct_v0& prev) {
        intField = prev.intField;
        floatField = prev.floatField;
        stringField = prev.stringField;
        vectorField = {1, 2, 3};
        optionalField = 99.99;
    }

    void ssDowngradeTo(ComplexTestStruct_v0& next) const {
        next.intField = intField;
        next.floatField = floatField;
        next.stringField = stringField;
    }

    void ssConvertFrom(const ComplexTestStruct_v0& prev) {
        ssUpgradeFrom(prev);
    }

    SS_COMPARISONS_MEMBER(ComplexTestStruct_v1)
};

} // namespace

#ifdef GENERATE_MODE
// Test for generating new format (F1) buffers that should cause integrity exceptions on old commit
TEST(SuitableStruct, NewFormatIntegrityCheck_GenerateNewFormatBuffers)
{
    SimpleTestStruct simpleData;
    simpleData.value = 42;
    simpleData.text = "new_format_test";
    simpleData.flag = true;

    ComplexTestStruct_v1 complexData;
    complexData.intField = 123;
    complexData.floatField = 3.14159f;
    complexData.stringField = "complex_new_format_test";
    complexData.vectorField = {10, 20, 30, 40, 50};
    complexData.optionalField = 2.71828;

    const auto simpleBuffer = ssSave(simpleData);
    const auto complexBuffer = ssSave(complexData);

    std::cout << "=== GENERATE MODE: Buffer data for new format (F1) integrity check ===" << std::endl;
    std::cout << "Simple buffer size: " << simpleBuffer.size() << " bytes" << std::endl;
    printBufferAsArray(simpleBuffer, "newFormatF1BufferData_simple");

    std::cout << "Complex buffer size: " << complexBuffer.size() << " bytes" << std::endl;
    printBufferAsArray(complexBuffer, "newFormatF1BufferData_complex");

    // Verify that we can load what we just generated (sanity check)
    SimpleTestStruct simpleLoaded;
    ComplexTestStruct_v1 complexLoaded;
    ssLoad(simpleBuffer, simpleLoaded);
    ssLoad(complexBuffer, complexLoaded);

    ASSERT_EQ(simpleData, simpleLoaded);
    ASSERT_EQ(complexData, complexLoaded);

    std::cout << "=== Generation completed successfully ===" << std::endl;
}
#endif // GENERATE_MODE

#ifndef GENERATE_MODE
// Test that new format (F1) buffers cause integrity exceptions when loaded with old implementation
TEST(SuitableStruct, NewFormatIntegrityCheck_SimpleStructShouldThrowException)
{
    Buffer newFormatBuffer(newFormatF1BufferData_simple, sizeof(newFormatF1BufferData_simple));
    SimpleTestStruct loaded;

    // This should throw std::exception because old implementation uses different hash function
    // and cannot verify the integrity of new format data
    EXPECT_THROW(ssLoad(newFormatBuffer, loaded), std::exception);
}

TEST(SuitableStruct, NewFormatIntegrityCheck_ComplexStructShouldThrowException)
{
    Buffer newFormatBuffer(newFormatF1BufferData_complex, sizeof(newFormatF1BufferData_complex));
    ComplexTestStruct_v1 loaded;
    EXPECT_THROW(ssLoad(newFormatBuffer, loaded), std::exception);
}

// Test that the buffers are not empty (sanity check)
TEST(SuitableStruct, NewFormatIntegrityCheck_BuffersAreNotEmpty)
{
    ASSERT_GT(sizeof(newFormatF1BufferData_simple), 16) << "Simple buffer seems to contain only placeholder data";
    ASSERT_GT(sizeof(newFormatF1BufferData_complex), 16) << "Complex buffer seems to contain only placeholder data";
}

// Test that buffers have correct new format markers
TEST(SuitableStruct, NewFormatIntegrityCheck_BuffersHaveCorrectFormatMarkers)
{
    // Check that buffers start with proper header structure
    // New format should have size (8 bytes) + hash (4 bytes) + format marker (5 bytes)
    ASSERT_GE(sizeof(newFormatF1BufferData_simple), 17) << "Simple buffer too small for new format header";
    ASSERT_GE(sizeof(newFormatF1BufferData_complex), 17) << "Complex buffer too small for new format header";

    // The format marker should be at offset 12 (after size and hash)
    // New format marker is {1, 0, 0, 0, 0}
    const uint8_t* simpleData = newFormatF1BufferData_simple;
    const uint8_t* complexData = newFormatF1BufferData_complex;

    // Skip size (8 bytes) and hash (4 bytes) to get to format marker
    const size_t formatMarkerOffset = 12;

    if (sizeof(newFormatF1BufferData_simple) > formatMarkerOffset + 4) {
        EXPECT_EQ(simpleData[formatMarkerOffset], 1) << "Simple buffer should have new format marker";
        EXPECT_EQ(simpleData[formatMarkerOffset + 1], 0);
        EXPECT_EQ(simpleData[formatMarkerOffset + 2], 0);
        EXPECT_EQ(simpleData[formatMarkerOffset + 3], 0);
        EXPECT_EQ(simpleData[formatMarkerOffset + 4], 0);
    }

    if (sizeof(newFormatF1BufferData_complex) > formatMarkerOffset + 4) {
        EXPECT_EQ(complexData[formatMarkerOffset], 1) << "Complex buffer should have new format marker";
        EXPECT_EQ(complexData[formatMarkerOffset + 1], 0);
        EXPECT_EQ(complexData[formatMarkerOffset + 2], 0);
        EXPECT_EQ(complexData[formatMarkerOffset + 3], 0);
        EXPECT_EQ(complexData[formatMarkerOffset + 4], 0);
    }
}
#endif // !GENERATE_MODE

#endif // 0
