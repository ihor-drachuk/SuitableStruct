/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY

// Uncomment the line below to switch to GENERATE mode
//#define GENERATE_MODE

#include <gtest/gtest.h>
#include <SuitableStruct/SerializerJson.h>
#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Internals/Exceptions.h>
#include <SuitableStruct/Containers/vector.h>
#include <vector>
#include <optional>
#include <chrono>
#include <variant>
#include <iostream>
#include "utils/time_utils.h"

#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

using namespace SuitableStruct;
using SuitableStructTest::toSeconds;

namespace {

constexpr auto ReferenceDurationRawSeconds = 1698408600;

#ifndef GENERATE_MODE
const char* legacyJsonData_v0 = R"json(
{
    "content": {
        "content": {
            "boolField": true,
            "floatField": 3.140000104904175,
            "intField": 42,
            "nestedField": {
                "content": {
                    "nestedInt": 111,
                    "nestedString": {
                        "content": "nested_v0",
                        "version": 0
                    }
                },
                "version": 0
            },
            "optionalField": {
                "content": {
                    "content": 123,
                    "hasValue": true
                },
                "version": 0
            },
            "stringField": {
                "content": "test_string_v0",
                "version": 0
            }
        },
        "version": 0
    },
    "hash": "769446932"
}
)json";

const char* legacyJsonData_v1 = R"json(
{
    "content": {
        "content": {
            "boolField": true,
            "doubleField": 2.718,
            "durationField": {
                "content": 500,
                "version": 0
            },
            "floatField": 3.140000104904175,
            "intField": 42,
            "jsonField": {
                "content": {
                    "key1": "value1",
                    "key2": 42
                },
                "version": 0
            },
            "nestedField": {
                "content": {
                    "nestedInt": 222,
                    "nestedString": {
                        "content": "nested_v1",
                        "version": 0
                    }
                },
                "version": 0
            },
            "optionalField": {
                "content": {
                    "content": 123,
                    "hasValue": true
                },
                "version": 0
            },
            "stringField": {
                "content": "test_string_v1",
                "version": 0
            },
            "vectorField": {
                "content": [
                    1,
                    2,
                    3,
                    4,
                    5
                ],
                "version": 0
            }
        },
        "version": 1
    },
    "hash": "1236429990"
}
)json";

const char* legacyJsonData_v2 = R"json(
{
    "content": {
        "content": {
            "boolField": true,
            "doubleField": 2.718,
            "durationField": {
                "content": 500,
                "version": 0
            },
            "floatField": 3.140000104904175,
            "intField": 42,
            "jsonField": {
                "content": {
                    "key1": "value1",
                    "key2": 42
                },
                "version": 0
            },
            "nestedField": {
                "content": {
                    "nestedFloat": 33.29999923706055,
                    "nestedInt": 333,
                    "nestedString": {
                        "content": "nested_v2",
                        "version": 0
                    }
                },
                "version": 0
            },
            "optionalField": {
                "content": {
                    "content": 123,
                    "hasValue": true
                },
                "version": 0
            },
            "optionalStringField": {
                "content": {
                    "content": {
                        "content": "optional_test",
                        "version": 0
                    },
                    "hasValue": true
                },
                "version": 0
            },
            "stringField": {
                "content": "test_string_v2",
                "version": 0
            },
            "timePointField": {
                "content": {
                    "Timepoint_version_marker": "0xFFFFFFFFFFFFFFFF52810BD50C38E940",
                    "data": {
                        "content": "1698408600000000700",
                        "version": 0
                    }
                },
                "version": 0
            },
            "variantField": {
                "content": {
                    "index": 2,
                    "value": {
                        "content": "variant_test",
                        "version": 0
                    }
                },
                "version": 0
            },
            "vectorField": {
                "content": [
                    1,
                    2,
                    3,
                    4,
                    5
                ],
                "version": 0
            }
        },
        "version": 2
    },
    "hash": "1796046013"
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
    std::string jsonString = doc.toJson(QJsonDocument::Indented).toStdString();

    std::cout << "const char* " << name << " = R\"json(" << std::endl;
    std::cout << jsonString;
    std::cout << ")json\";" << std::endl << std::endl;
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
    auto ssNamesTuple() const { return std::tie("nestedInt", "nestedString"); }
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
    auto ssNamesTuple() const { return std::tie("nestedInt", "nestedString", "nestedFloat"); }
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
    auto ssNamesTuple() const { return std::tie("intField", "floatField", "stringField", "boolField", "optionalField", "nestedField"); }

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
    QJsonValue jsonField;

    auto ssTuple() const {
        return std::tie(intField, floatField, stringField, boolField, optionalField, nestedField,
                       doubleField, vectorField, durationField, jsonField);
    }
    auto ssNamesTuple() const {
        return std::tie("intField", "floatField", "stringField", "boolField", "optionalField", "nestedField",
                       "doubleField", "vectorField", "durationField", "jsonField");
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
        jsonField = QJsonObject({{"upgraded", true}});
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
    QJsonValue jsonField;

    // New fields in v2
    std::variant<std::monostate, int, std::string> variantField;
    std::chrono::steady_clock::time_point timePointField;
    std::optional<std::string> optionalStringField;

    auto ssTuple() const {
        return std::tie(intField, floatField, stringField, boolField, optionalField, nestedField,
                       doubleField, vectorField, durationField, jsonField,
                       variantField, timePointField, optionalStringField);
    }
    auto ssNamesTuple() const {
        return std::tie("intField", "floatField", "stringField", "boolField", "optionalField", "nestedField",
                       "doubleField", "vectorField", "durationField", "jsonField",
                       "variantField", "timePointField", "optionalStringField");
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
// Test for serializing structures (to be used for generating legacy JSON const char* on old commit)
TEST(SuitableStruct, JsonLegacyFormatF0CompatibilityTest_P_SerializeStructures)
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
    v1_value.jsonField = QJsonObject({{"key1", "value1"}, {"key2", 42}});

    // Test data for v2
    ComplexStruct_v2 v2_value;
    v2_value.intField = 42;
    v2_value.floatField = 3.14f;
    v2_value.stringField = "test_string_v2";
    v2_value.boolField = true;
    v2_value.optionalField = 123;
    v2_value.nestedField.nestedInt = 333;
    v2_value.nestedField.nestedString = "nested_v2";
    v2_value.nestedField.nestedFloat = 33.3f;
    v2_value.doubleField = 2.718;
    v2_value.vectorField = {1, 2, 3, 4, 5};
    v2_value.durationField = std::chrono::milliseconds(500);
    v2_value.jsonField = QJsonObject({{"key1", "value1"}, {"key2", 42}});
    v2_value.variantField = std::string("variant_test");
    v2_value.timePointField = SuitableStructTest::timeToSteadyClock(ReferenceDurationRawSeconds);
    v2_value.optionalStringField = "optional_test";


    // Serialize and verify they work in current format
    const auto v0_json = ssJsonSave(v0_value);
    const auto v1_json = ssJsonSave(v1_value);
    const auto v2_json = ssJsonSave(v2_value);

    // GENERATE MODE: Print JSON for copying into legacy const char* arrays
    std::cout << "=== GENERATE MODE: JSON data for legacy compatibility ===" << std::endl;
    printJsonAsConstChar(v0_json, "legacyJsonData_v0");
    printJsonAsConstChar(v1_json, "legacyJsonData_v1");
    printJsonAsConstChar(v2_json, "legacyJsonData_v2");

    // Test that we can load what we just generated
    ComplexStruct_v0 v0_loaded;
    ComplexStruct_v1 v1_loaded;
    ComplexStruct_v2 v2_loaded;
    ssJsonLoad(v0_json, v0_loaded);
    ssJsonLoad(v1_json, v1_loaded);
    ssJsonLoad(v2_json, v2_loaded);

    // Test that we can load what we just generated
    ASSERT_EQ(v0_value, v0_loaded);
    ASSERT_EQ(v1_value, v1_loaded);
    ASSERT_EQ(v2_value, v2_loaded);
}
#endif // GENERATE_MODE

#ifndef GENERATE_MODE
// From legacy JSON, data v0 -> v0
TEST(SuitableStruct, JsonLegacyFormatF0CompatibilityTest_P_LoadJson_v0_to_v0)
{
    const QJsonValue legacyJson = QJsonDocument::fromJson(legacyJsonData_v0).object();
    ComplexStruct_v0 loaded;
    ssJsonLoad(legacyJson, loaded);

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

// From legacy JSON, data v1 -> v0 (should fail - downgrade not supported by default in JSON, fields missing)
TEST(SuitableStruct, JsonLegacyFormatF0CompatibilityTest_P_LoadJson_v1_to_v0_ShouldFail)
{
    const QJsonValue legacyJson = QJsonDocument::fromJson(legacyJsonData_v1).object();
    ComplexStruct_v0 loaded;

    // This should fail because v0 cannot load v1 data if fields are missing or types mismatch without ssDowngradeTo
    EXPECT_THROW(ssJsonLoad(legacyJson, loaded), std::exception);
}

// From legacy JSON, data v0 -> v1 (upgrade)
TEST(SuitableStruct, JsonLegacyFormatF0CompatibilityTest_P_LoadJson_v0_to_v1)
{
    const QJsonValue legacyJson = QJsonDocument::fromJson(legacyJsonData_v0).object();
    ComplexStruct_v1 loaded;
    ssJsonLoad(legacyJson, loaded);

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
    ASSERT_TRUE(loaded.jsonField.isObject());
    ASSERT_TRUE(loaded.jsonField.toObject()["upgraded"].toBool());
}

// From legacy JSON, data v1 -> v1
TEST(SuitableStruct, JsonLegacyFormatF0CompatibilityTest_P_LoadJson_v1_to_v1)
{
    const QJsonValue legacyJson = QJsonDocument::fromJson(legacyJsonData_v1).object();
    ComplexStruct_v1 loaded;
    ssJsonLoad(legacyJson, loaded);

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
    ASSERT_TRUE(loaded.jsonField.isObject());
    ASSERT_EQ(loaded.jsonField.toObject()["key1"].toString().toStdString(), "value1");
    ASSERT_EQ(loaded.jsonField.toObject()["key2"].toInt(), 42);
}

// From legacy JSON, data v0 -> v2 (upgrade)
TEST(SuitableStruct, JsonLegacyFormatF0CompatibilityTest_P_LoadJson_v0_to_v2)
{
    const QJsonValue legacyJson = QJsonDocument::fromJson(legacyJsonData_v0).object();
    ComplexStruct_v2 loaded;
    ssJsonLoad(legacyJson, loaded);

    // Verify the loaded data matches expected values from v0
    ASSERT_EQ(loaded.intField, 42);
    ASSERT_FLOAT_EQ(loaded.floatField, 3.14f);
    ASSERT_EQ(loaded.stringField, "test_string_v0");
    ASSERT_TRUE(loaded.boolField);
    ASSERT_TRUE(loaded.optionalField.has_value());
    ASSERT_EQ(loaded.optionalField.value(), 123);
    ASSERT_EQ(loaded.nestedField.nestedInt, 111);
    ASSERT_EQ(loaded.nestedField.nestedString, "nested_v0");
    ASSERT_FLOAT_EQ(loaded.nestedField.nestedFloat, 1.553f); // Upgraded in NestedStruct_v2

    // Verify v1-specific fields have upgrade values (v0->v1 upgrade)
    ASSERT_DOUBLE_EQ(loaded.doubleField, 99.99);
    ASSERT_EQ(loaded.vectorField, std::vector<int>({100, 200}));
    ASSERT_EQ(loaded.durationField, std::chrono::milliseconds(1000));
    ASSERT_TRUE(loaded.jsonField.isObject());
    ASSERT_TRUE(loaded.jsonField.toObject()["upgraded"].toBool());


    // Verify v2-specific fields have upgrade values (v0->v1 upgrade)
    ASSERT_TRUE(std::holds_alternative<std::string>(loaded.variantField));
    ASSERT_EQ(std::get<std::string>(loaded.variantField), "upgraded_v2");
    ASSERT_NEAR(toSeconds(loaded.timePointField), ReferenceDurationRawSeconds, 1.0);
    ASSERT_TRUE(loaded.optionalStringField.has_value());
    ASSERT_EQ(loaded.optionalStringField.value(), "upgraded_optional");
}

// From legacy JSON, data v1 -> v2 (upgrade)
TEST(SuitableStruct, JsonLegacyFormatF0CompatibilityTest_P_LoadJson_v1_to_v2)
{
    const QJsonValue legacyJson = QJsonDocument::fromJson(legacyJsonData_v1).object();
    ComplexStruct_v2 loaded;
    ssJsonLoad(legacyJson, loaded);

    // Verify the loaded data matches expected values from v1
    ASSERT_EQ(loaded.intField, 42);
    ASSERT_FLOAT_EQ(loaded.floatField, 3.14f);
    ASSERT_EQ(loaded.stringField, "test_string_v1");
    ASSERT_TRUE(loaded.boolField);
    ASSERT_TRUE(loaded.optionalField.has_value());
    ASSERT_EQ(loaded.optionalField.value(), 123);
    ASSERT_EQ(loaded.nestedField.nestedInt, 222);
    ASSERT_EQ(loaded.nestedField.nestedString, "nested_v1");
    ASSERT_FLOAT_EQ(loaded.nestedField.nestedFloat, 1.553f); // Upgraded in NestedStruct_v2
    ASSERT_DOUBLE_EQ(loaded.doubleField, 2.718);
    ASSERT_EQ(loaded.vectorField, std::vector<int>({1, 2, 3, 4, 5}));
    ASSERT_EQ(loaded.durationField, std::chrono::milliseconds(500));
    ASSERT_TRUE(loaded.jsonField.isObject());
    ASSERT_EQ(loaded.jsonField.toObject()["key1"].toString().toStdString(), "value1");
    ASSERT_EQ(loaded.jsonField.toObject()["key2"].toInt(), 42);

    // Verify v2-specific fields have upgrade values (v0->v1 upgrade)
    ASSERT_TRUE(std::holds_alternative<std::string>(loaded.variantField));
    ASSERT_EQ(std::get<std::string>(loaded.variantField), "upgraded_v2");
    ASSERT_NEAR(toSeconds(loaded.timePointField), ReferenceDurationRawSeconds, 1.0);
    ASSERT_TRUE(loaded.optionalStringField.has_value());
    ASSERT_EQ(loaded.optionalStringField.value(), "upgraded_optional");
}

// From legacy JSON, data v2 -> v2
TEST(SuitableStruct, JsonLegacyFormatF0CompatibilityTest_P_LoadJson_v2_to_v2)
{
    const QJsonValue legacyJson = QJsonDocument::fromJson(legacyJsonData_v2).object();
    ComplexStruct_v2 loaded;
    ssJsonLoad(legacyJson, loaded);

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
    ASSERT_TRUE(loaded.jsonField.isObject());
    ASSERT_EQ(loaded.jsonField.toObject()["key1"].toString().toStdString(), "value1");
    ASSERT_EQ(loaded.jsonField.toObject()["key2"].toInt(), 42);
    ASSERT_TRUE(std::holds_alternative<std::string>(loaded.variantField));
    ASSERT_EQ(std::get<std::string>(loaded.variantField), "variant_test");
    ASSERT_NEAR(toSeconds(loaded.timePointField), ReferenceDurationRawSeconds, 1.0);
    ASSERT_TRUE(loaded.optionalStringField.has_value());
    ASSERT_EQ(loaded.optionalStringField.value(), "optional_test");
}

// From legacy JSON, data v2 -> v0 (should fail)
TEST(SuitableStruct, JsonLegacyFormatF0CompatibilityTest_P_LoadJson_v2_to_v0_ShouldFail)
{
    const QJsonValue legacyJson = QJsonDocument::fromJson(legacyJsonData_v2).object();
    ComplexStruct_v0 loaded;
    EXPECT_THROW(ssJsonLoad(legacyJson, loaded), std::exception);
}

// From legacy JSON, data v2 -> v1 (should fail)
TEST(SuitableStruct, JsonLegacyFormatF0CompatibilityTest_P_LoadJson_v2_to_v1_ShouldFail)
{
    const QJsonValue legacyJson = QJsonDocument::fromJson(legacyJsonData_v2).object();
    ComplexStruct_v1 loaded;
    EXPECT_THROW(ssJsonLoad(legacyJson, loaded), std::exception);
}

// Test current format compatibility - ensure new format works correctly
TEST(SuitableStruct, JsonLegacyFormatF0CompatibilityTest_P_CurrentFormat)
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
    original.jsonField = QJsonObject({{"key1", "value1"}, {"key2", 42}});
    original.variantField = std::string("variant_test");
    original.timePointField = SuitableStructTest::timeToSteadyClock(ReferenceDurationRawSeconds);
    original.optionalStringField = "optional_test";

    // Test serialization and deserialization
    const auto jsonVal = ssJsonSave(original);
    ComplexStruct_v2 loaded;
    ssJsonLoad(jsonVal, loaded);
    ASSERT_EQ(original, loaded);
}

// Test version downgrade compatibility (JSON)
TEST(SuitableStruct, JsonLegacyFormatF0CompatibilityTest_P_VersionDowngrade)
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
    v2.jsonField = QJsonValue("test_json_val");
    v2.variantField = std::string("test_variant");
    v2.timePointField = SuitableStructTest::timeToSteadyClock(ReferenceDurationRawSeconds);
    v2.optionalStringField = "test_optional";

    // Save v2 and load as v1
    const auto json_v2 = ssJsonSave(v2);
    ComplexStruct_v1 v1;
    ssJsonLoad(json_v2, v1); // Uses ssDowngradeTo for ComplexStruct_v2 -> ComplexStruct_v1

    // Verify common fields are preserved (NestedStruct also downgrades)
    ASSERT_EQ(v1.intField, v2.intField);
    ASSERT_FLOAT_EQ(v1.floatField, v2.floatField);
    ASSERT_EQ(v1.stringField, v2.stringField);
    ASSERT_EQ(v1.boolField, v2.boolField);
    ASSERT_EQ(v1.optionalField, v2.optionalField);
    ASSERT_EQ(v1.nestedField.nestedInt, v2.nestedField.nestedInt); // NestedStruct_v01 from NestedStruct_v2
    ASSERT_EQ(v1.nestedField.nestedString, v2.nestedField.nestedString);
    ASSERT_DOUBLE_EQ(v1.doubleField, v2.doubleField);
    ASSERT_EQ(v1.vectorField, v2.vectorField);
    ASSERT_EQ(v1.durationField, v2.durationField);
    ASSERT_EQ(v1.jsonField, v2.jsonField);


    // Save v1 and load as v0
    const auto json_v1 = ssJsonSave(v1);
    ComplexStruct_v0 v0;
    ssJsonLoad(json_v1, v0); // Uses ssDowngradeTo for ComplexStruct_v1 -> ComplexStruct_v0

    // Verify common fields are preserved
    ASSERT_EQ(v0.intField, v1.intField);
    ASSERT_FLOAT_EQ(v0.floatField, v1.floatField);
    ASSERT_EQ(v0.stringField, v1.stringField);
    ASSERT_EQ(v0.boolField, v1.boolField);
    ASSERT_EQ(v0.optionalField, v1.optionalField);
    ASSERT_EQ(v0.nestedField, v1.nestedField); // v01 to v01, direct copy
}
#endif // !GENERATE_MODE

#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
