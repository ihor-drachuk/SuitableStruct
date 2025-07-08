/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>
#include <string>
#include <optional>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QJsonValue>
#include <QJsonObject>
#include <SuitableStruct/SerializerJson.h>
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

using namespace SuitableStruct;

namespace {

struct SimpleTestStruct
{
    int value {};
    std::string text;
    bool flag {};

    using ssVersions = std::tuple<SimpleTestStruct>;
    auto ssTuple() const { return std::tie(value, text, flag); }
    auto ssNamesTuple() const { return std::tie("value", "text", "flag"); }

    SS_COMPARISONS_MEMBER(SimpleTestStruct)
};

} // namespace

TEST(SuitableStruct, FormatDetection_DetectF1FormatFromCurrentSerialization)
{
    // Create test data
    SimpleTestStruct testData;
    testData.value = 42;
    testData.text = "format_test";
    testData.flag = true;

    // Serialize with current implementation (should be F1)
    const auto buffer = ssSave(testData);

    // Detect format
    const auto optDetectedFormat = ssDetectFormat(buffer);
    EXPECT_EQ(optDetectedFormat, SSDataFormat::F1);
}

TEST(SuitableStruct, FormatDetection_UnknownFormatForInvalidData)
{
    // Test with too small buffer
    const uint8_t tooSmallData[] = {0x01, 0x02, 0x03};
    const Buffer tooSmallBuffer(tooSmallData, sizeof(tooSmallData));

    EXPECT_FALSE(ssDetectFormat(tooSmallBuffer).has_value());

    // Test with invalid hash
    uint8_t invalidHashData[50];
    memset(invalidHashData, 0xFF, sizeof(invalidHashData));
    const Buffer invalidHashBuffer(invalidHashData, sizeof(invalidHashData));

    EXPECT_FALSE(ssDetectFormat(invalidHashBuffer).has_value());

    // Test with empty buffer
    const Buffer emptyBuffer;

    EXPECT_FALSE(ssDetectFormat(emptyBuffer).has_value());
}

TEST(SuitableStruct, FormatDetection_BufferReaderPositionPreserved)
{
    // Create test data and serialize
    SimpleTestStruct testData;
    testData.value = 42;
    testData.text = "position_test";
    testData.flag = true;

    const auto modernBuffer = ssSave(testData);
    BufferReader reader(modernBuffer);

    // Advance to some position
    reader.advance(5);
    const size_t originalPosition = reader.position();

    // Detect format
    const auto optDetectedFormat = ssDetectFormat(reader);

    // Check that position is preserved
    EXPECT_EQ(reader.position(), originalPosition);
    EXPECT_FALSE(optDetectedFormat.has_value());
}

TEST(SuitableStruct, FormatDetection_MultipleStructTypes)
{
    // Test format detection with different struct types
    SimpleTestStruct simpleData;
    simpleData.value = 123;
    simpleData.text = "complex_test";
    simpleData.flag = true;

    const auto simpleBuffer = ssSave(simpleData);
    const auto optSimpleFormat = ssDetectFormat(simpleBuffer);
    EXPECT_EQ(optSimpleFormat, SSDataFormat::F1);
}

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
TEST(SuitableStruct, FormatDetection_DetectJsonF1Format)
{
    // Create test data and serialize to JSON
    SimpleTestStruct testData;
    testData.value = 42;
    testData.text = "json_format_test";
    testData.flag = true;

    // Serialize with current implementation (should be F1)
    const auto jsonValue = ssJsonSave(testData);

    // Detect format
    const auto optDetectedFormat = ssDetectJsonFormat(jsonValue);
    EXPECT_EQ(optDetectedFormat, SSDataFormat::F1);
}

TEST(SuitableStruct, FormatDetection_JsonUnknownFormatForInvalidData)
{
    // Test with non-object JSON
    const QJsonValue nonObjectJson("not_an_object");
    EXPECT_FALSE(ssDetectJsonFormat(nonObjectJson).has_value());

    // Test with object without format markers
    QJsonObject invalidObj;
    invalidObj["some_field"] = "some_value";
    EXPECT_FALSE(ssDetectJsonFormat(invalidObj).has_value());

    // Test with invalid F1 format (missing required fields)
    QJsonObject invalidF1;
    invalidF1["ss_format_version"] = "1.0";
    invalidF1["hash"] = 12345; // Missing segments
    EXPECT_FALSE(ssDetectJsonFormat(invalidF1).has_value());

    // Test with invalid hash in F0 format
    QJsonObject invalidF0;
    invalidF0["hash"] = 99999; // Wrong hash
    invalidF0["content"] = QJsonObject();
    EXPECT_FALSE(ssDetectJsonFormat(invalidF0).has_value());
}

TEST(SuitableStruct, FormatDetection_JsonMultipleStructTypes)
{
    // Test format detection with different struct types
    SimpleTestStruct simpleData;
    simpleData.value = 123;
    simpleData.text = "simple_json_test";
    simpleData.flag = true;

    const auto simpleJsonValue = ssJsonSave(simpleData);
    const auto optSimpleFormat = ssDetectJsonFormat(simpleJsonValue);
    EXPECT_EQ(optSimpleFormat, SSDataFormat::F1);
}
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
