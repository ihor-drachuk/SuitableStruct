/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY

// Replace by "1" to enable test/generator
#if 0

// Uncomment to switch to GENERATE mode
#define GENERATE_MODE

#include <gtest/gtest.h>
#include <SuitableStruct/SerializerJson.h>
#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Containers/vector.h>
#include <vector>
#include <optional>
#include <iostream>

#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

using namespace SuitableStruct;

namespace {

#ifndef GENERATE_MODE
// These JSON strings contain data serialized with new format that should cause
// integrity exceptions when loaded with old implementation due to hash function changes
const char* newFormatJsonData_simple = R"json(
{
    "hash": "2929779296",
    "segments": [
        {
            "data": {
                "flag": true,
                "text": [
                    {
                        "data": "new_json_format_test",
                        "version_index": 0
                    }
                ],
                "value": 42
            },
            "version_index": 0
        }
    ],
    "ss_format_version": "1.0"
}
)json";

const char* newFormatJsonData_complex = R"json(
{
    "hash": "115093127",
    "segments": [
        {
            "data": {
                "floatField": 3.141590118408203,
                "intField": 123,
                "optionalField": [
                    {
                        "data": {
                            "content": 2.71828,
                            "hasValue": true
                        },
                        "version_index": 0
                    }
                ],
                "stringField": [
                    {
                        "data": "complex_new_json_format_test",
                        "version_index": 0
                    }
                ],
                "vectorField": [
                    {
                        "data": [
                            10,
                            20,
                            30,
                            40,
                            50
                        ],
                        "version_index": 0
                    }
                ]
            },
            "version_index": 1
        },
        {
            "data": {
                "floatField": 3.141590118408203,
                "intField": 123,
                "stringField": [
                    {
                        "data": "complex_new_json_format_test",
                        "version_index": 0
                    }
                ]
            },
            "version_index": 0
        }
    ],
    "ss_format_version": "1.0"
}
)json";
#endif // !GENERATE_MODE

#ifdef GENERATE_MODE
// Function to print QJsonValue as string
void printJsonAsConstChar(const QJsonValue& jsonValue, const std::string& name)
{
    auto doc = [&]() -> QJsonDocument {
        if (jsonValue.isObject()) {
            return QJsonDocument(jsonValue.toObject());
        } else if (jsonValue.isArray()) {
            return QJsonDocument(jsonValue.toArray());
        } else {
            assert(false && "Unexpected value!");
            return {};
        }
    }();
    const std::string jsonString = doc.toJson(QJsonDocument::Indented).toStdString();

    std::cout << "const char* " << name << " = R\"json(" << std::endl;
    std::cout << jsonString;
    std::cout << ")json\";" << std::endl << std::endl;
}
#endif // GENERATE_MODE

// Simple test structure
struct SimpleTestStruct
{
    int value {};
    std::string text;
    bool flag {};

    auto ssTuple() const { return std::tie(value, text, flag); }
    auto ssNamesTuple() const { return std::tie("value", "text", "flag"); }
    SS_COMPARISONS_MEMBER(SimpleTestStruct)
};

// Complex test structure with versioning
struct ComplexTestStruct_v0
{
    int intField {};
    float floatField {};
    std::string stringField;

    using ssVersions = std::tuple<ComplexTestStruct_v0>;
    auto ssTuple() const { return std::tie(intField, floatField, stringField); }
    auto ssNamesTuple() const { return std::tie("intField", "floatField", "stringField"); }
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
    auto ssNamesTuple() const { return std::tie("intField", "floatField", "stringField", "vectorField", "optionalField"); }

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
// Test for generating new format JSON that should cause integrity exceptions on old commit
TEST(SuitableStruct, NewJsonFormatIntegrityCheck_GenerateNewFormatJson)
{
    SimpleTestStruct simpleData;
    simpleData.value = 42;
    simpleData.text = "new_json_format_test";
    simpleData.flag = true;

    ComplexTestStruct_v1 complexData;
    complexData.intField = 123;
    complexData.floatField = 3.14159f;
    complexData.stringField = "complex_new_json_format_test";
    complexData.vectorField = {10, 20, 30, 40, 50};
    complexData.optionalField = 2.71828;

    // Serialize using new JSON format
    const auto simpleJson = ssJsonSave(simpleData);
    const auto complexJson = ssJsonSave(complexData);

    std::cout << "=== GENERATE MODE: JSON data for new format integrity check ===" << std::endl;
    printJsonAsConstChar(simpleJson, "newFormatJsonData_simple");
    printJsonAsConstChar(complexJson, "newFormatJsonData_complex");

    // Verify that we can load what we just generated (sanity check)
    SimpleTestStruct simpleLoaded;
    ComplexTestStruct_v1 complexLoaded;
    ssJsonLoad(simpleJson, simpleLoaded);
    ssJsonLoad(complexJson, complexLoaded);

    ASSERT_EQ(simpleData, simpleLoaded);
    ASSERT_EQ(complexData, complexLoaded);

    std::cout << "=== JSON Generation completed successfully ===" << std::endl;
}
#endif // GENERATE_MODE

#ifndef GENERATE_MODE
// Test that new format JSON causes integrity exceptions when loaded with old implementation
TEST(SuitableStruct, NewJsonFormatIntegrityCheck_SimpleStructShouldThrowException)
{
    QJsonValue newFormatJson = QJsonDocument::fromJson(newFormatJsonData_simple).object();
    SimpleTestStruct loaded;

    // This should throw std::exception because old implementation uses different hash function
    // and cannot verify the integrity of new format JSON data
    EXPECT_THROW(ssJsonLoad(newFormatJson, loaded), std::exception);
}

TEST(SuitableStruct, NewJsonFormatIntegrityCheck_ComplexStructShouldThrowException)
{
    QJsonValue newFormatJson = QJsonDocument::fromJson(newFormatJsonData_complex).object();
    ComplexTestStruct_v1 loaded;
    EXPECT_THROW(ssJsonLoad(newFormatJson, loaded), std::exception);
}

// Test that the JSON data is not empty (sanity check)
TEST(SuitableStruct, NewJsonFormatIntegrityCheck_JsonDataIsNotEmpty)
{
    QJsonValue simpleJson = QJsonDocument::fromJson(newFormatJsonData_simple).object();
    QJsonValue complexJson = QJsonDocument::fromJson(newFormatJsonData_complex).object();

    ASSERT_TRUE(simpleJson.isObject()) << "Simple JSON should be a valid object";
    ASSERT_TRUE(complexJson.isObject()) << "Complex JSON should be a valid object";

    // Check that it's not just placeholder data
    ASSERT_FALSE(simpleJson.toObject().contains("placeholder")) << "Simple JSON seems to contain only placeholder data";
    ASSERT_FALSE(complexJson.toObject().contains("placeholder")) << "Complex JSON seems to contain only placeholder data";
}

// Test that JSON has correct new format structure
TEST(SuitableStruct, NewJsonFormatIntegrityCheck_JsonHasCorrectNewFormatStructure)
{
    QJsonValue simpleJson = QJsonDocument::fromJson(newFormatJsonData_simple).object();
    QJsonValue complexJson = QJsonDocument::fromJson(newFormatJsonData_complex).object();

    ASSERT_TRUE(simpleJson.isObject());
    ASSERT_TRUE(complexJson.isObject());

    const auto simpleObj = simpleJson.toObject();
    const auto complexObj = complexJson.toObject();

    // New format should have ss_format_version field
    EXPECT_TRUE(simpleObj.contains("ss_format_version")) << "Simple JSON should have new format version field";
    EXPECT_TRUE(complexObj.contains("ss_format_version")) << "Complex JSON should have new format version field";

    if (simpleObj.contains("ss_format_version")) {
        EXPECT_EQ(simpleObj["ss_format_version"].toString().toStdString(), "1.0") << "Simple JSON should have format version 1.0";
    }

    if (complexObj.contains("ss_format_version")) {
        EXPECT_EQ(complexObj["ss_format_version"].toString().toStdString(), "1.0") << "Complex JSON should have format version 1.0";
    }

    // New format should have segments field
    EXPECT_TRUE(simpleObj.contains("segments")) << "Simple JSON should have segments field";
    EXPECT_TRUE(complexObj.contains("segments")) << "Complex JSON should have segments field";

    // New format should have hash field
    EXPECT_TRUE(simpleObj.contains("hash")) << "Simple JSON should have hash field";
    EXPECT_TRUE(complexObj.contains("hash")) << "Complex JSON should have hash field";
}
#endif // !GENERATE_MODE

#endif // 0

#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
