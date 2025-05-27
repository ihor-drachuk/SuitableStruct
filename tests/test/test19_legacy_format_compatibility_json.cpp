/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <SuitableStruct/SerializerJson.h>
#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Exceptions.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace SuitableStruct;

namespace {

// Test structure with 3 versions for JSON legacy format compatibility testing
struct JsonTestStruct_v0
{
    int value {};

    using ssVersions = std::tuple<JsonTestStruct_v0>;
    auto ssTuple() const { return std::tie(value); }
    auto ssNamesTuple() const { return std::tie("value"); }
    SS_COMPARISONS_MEMBER(JsonTestStruct_v0)
};

struct JsonTestStruct_v1
{
    int value {};
    float extra {};

    using ssVersions = std::tuple<JsonTestStruct_v0, JsonTestStruct_v1>;
    auto ssTuple() const { return std::tie(value, extra); }
    auto ssNamesTuple() const { return std::tie("value", "extra"); }

    void ssUpgradeFrom(const JsonTestStruct_v0& prev) {
        value = prev.value;
        extra = 2.5f;
    }

    void ssDowngradeTo(JsonTestStruct_v0& next) const {
        next.value = value;
    }

    SS_COMPARISONS_MEMBER(JsonTestStruct_v1)
};

struct JsonTestStruct_v2
{
    int value {};
    double extra {};
    std::string name;

    using ssVersions = std::tuple<JsonTestStruct_v0, JsonTestStruct_v1, JsonTestStruct_v2>;
    auto ssTuple() const { return std::tie(value, extra, name); }
    auto ssNamesTuple() const { return std::tie("value", "extra", "name"); }

    void ssUpgradeFrom(const JsonTestStruct_v1& prev) {
        value = prev.value;
        extra = static_cast<double>(prev.extra);
        name = "upgraded_v2";
    }

    void ssDowngradeTo(JsonTestStruct_v1& next) const {
        next.value = value;
        next.extra = static_cast<float>(extra);
    }

    SS_COMPARISONS_MEMBER(JsonTestStruct_v2)
};

// Helper function to create legacy F0 format JSON manually
// Legacy format was: {"hash": hash_value, "content": {"version": X, "content": actual_data}}
QJsonValue createLegacyF0Json(const QJsonValue& data, int version)
{
    QJsonObject contentWrapper;
    contentWrapper["version"] = version;
    contentWrapper["content"] = data;

    QJsonObject root;
    root["hash"] = static_cast<int>(Internal::ssJsonHashValue_F0(contentWrapper)); // Legacy hash calculation
    root["content"] = contentWrapper;

    return root;
}

// Helper to create direct legacy data without version wrapper
// Some legacy formats might have been saved as direct {"hash": X, "content": data}
QJsonValue createLegacyDirectJson(const QJsonValue& data)
{
    QJsonObject root;
    root["hash"] = static_cast<int>(Internal::ssJsonHashValue_F0(data)); // Legacy hash calculation
    root["content"] = data;

    return root;
}

} // namespace

TEST(SuitableStruct, LegacyFormatCompatibilityJson_LoadJsonTestStructV0_SameVersion)
{
    JsonTestStruct_v0 original;
    original.value = 42;

    // Create legacy format manually
    QJsonObject data;
    data["value"] = original.value;
    const auto legacyJson = createLegacyF0Json(data, 0);

    JsonTestStruct_v0 loaded;
    ssJsonLoad(legacyJson, loaded);

    EXPECT_EQ(original, loaded);
}

TEST(SuitableStruct, LegacyFormatCompatibilityJson_LoadJsonTestStructV1_SameVersion)
{
    JsonTestStruct_v1 original;
    original.value = 42;
    original.extra = 3.14f;

    // Create legacy format manually
    QJsonObject data;
    data["value"] = original.value;
    data["extra"] = static_cast<double>(original.extra);
    const auto legacyJson = createLegacyF0Json(data, 1);

    JsonTestStruct_v1 loaded;
    ssJsonLoad(legacyJson, loaded);

    EXPECT_EQ(original, loaded);
}

TEST(SuitableStruct, LegacyFormatCompatibilityJson_LoadJsonTestStructV2_SameVersion)
{
    JsonTestStruct_v2 original;
    original.value = 42;
    original.extra = 3.14;
    original.name = "test_v2";

    // Create legacy format manually
    QJsonObject data;
    data["value"] = original.value;
    data["extra"] = original.extra;
    data["name"] = QString::fromStdString(original.name);
    const auto legacyJson = createLegacyF0Json(data, 2);

    JsonTestStruct_v2 loaded;
    ssJsonLoad(legacyJson, loaded);

    EXPECT_EQ(original, loaded);
}

TEST(SuitableStruct, LegacyFormatCompatibilityJson_UpgradeV0ToV1)
{
    // Create v0 struct and save it in legacy format
    JsonTestStruct_v0 original;
    original.value = 100;

    QJsonObject data;
    data["value"] = original.value;
    const auto legacyJson = createLegacyF0Json(data, 0);

    // Load it as v1 (should trigger upgrade)
    JsonTestStruct_v1 loaded;
    ssJsonLoad(legacyJson, loaded);

    EXPECT_EQ(loaded.value, 100);
    EXPECT_FLOAT_EQ(loaded.extra, 2.5f);  // Default value from upgrade
}

TEST(SuitableStruct, LegacyFormatCompatibilityJson_UpgradeV0ToV2)
{
    // Create v0 struct and save it in legacy format
    JsonTestStruct_v0 original;
    original.value = 200;

    QJsonObject data;
    data["value"] = original.value;
    const auto legacyJson = createLegacyF0Json(data, 0);

    // Load it as v2 (should trigger upgrade v0->v1->v2)
    JsonTestStruct_v2 loaded;
    ssJsonLoad(legacyJson, loaded);

    EXPECT_EQ(loaded.value, 200);
    EXPECT_DOUBLE_EQ(loaded.extra, 2.5);    // Upgraded from v1's default
    EXPECT_EQ(loaded.name, "upgraded_v2");  // Default value from v1->v2 upgrade
}

TEST(SuitableStruct, LegacyFormatCompatibilityJson_UpgradeV1ToV2)
{
    // Create v1 struct and save it in legacy format
    JsonTestStruct_v1 original;
    original.value = 300;
    original.extra = 7.5f;

    QJsonObject data;
    data["value"] = original.value;
    data["extra"] = static_cast<double>(original.extra);
    const auto legacyJson = createLegacyF0Json(data, 1);

    // Load it as v2 (should trigger upgrade)
    JsonTestStruct_v2 loaded;
    ssJsonLoad(legacyJson, loaded);

    EXPECT_EQ(loaded.value, 300);
    EXPECT_DOUBLE_EQ(loaded.extra, 7.5);     // Converted from float to double
    EXPECT_EQ(loaded.name, "upgraded_v2");   // Default value from upgrade
}

TEST(SuitableStruct, LegacyFormatCompatibilityJson_InvalidVersionShouldThrow)
{
    // Create v0 struct but mark it with invalid version
    JsonTestStruct_v0 original;
    original.value = 42;

    QJsonObject data;
    data["value"] = original.value;
    const auto legacyJson = createLegacyF0Json(data, 99);  // Invalid version

    // Should throw when trying to load
    JsonTestStruct_v0 loaded;
    EXPECT_THROW(ssJsonLoad(legacyJson, loaded), VersionError);
}

TEST(SuitableStruct, LegacyFormatCompatibilityJson_CorruptedLegacyJsonShouldThrow)
{
    // Test various corruption scenarios for legacy JSON format
    JsonTestStruct_v0 original;
    original.value = 42;

    QJsonObject data;
    data["value"] = original.value;

    // Test 1: Corrupt hash
    {
        const auto legacyJson = createLegacyF0Json(data, 0);
        auto rootObj = legacyJson.toObject();
        rootObj["hash"] = rootObj["hash"].toInt() + 1; // Corrupt hash
        const auto corruptedJson = QJsonValue(rootObj);

        JsonTestStruct_v0 loaded;
        EXPECT_THROW(ssJsonLoad(corruptedJson, loaded), IntegrityError);
    }

    // Test 2: Missing hash
    {
        const auto legacyJson = createLegacyF0Json(data, 0);
        auto rootObj = legacyJson.toObject();
        rootObj.remove("hash");
        const auto corruptedJson = QJsonValue(rootObj);

        JsonTestStruct_v0 loaded;
        EXPECT_THROW(ssJsonLoad(corruptedJson, loaded), FormatError);
    }

    // Test 3: Missing content
    {
        const auto legacyJson = createLegacyF0Json(data, 0);
        auto rootObj = legacyJson.toObject();
        rootObj.remove("content");
        const auto corruptedJson = QJsonValue(rootObj);

        JsonTestStruct_v0 loaded;
        EXPECT_THROW(ssJsonLoad(corruptedJson, loaded), FormatError);
    }

    // Test 4: Corrupt inner content structure
    {
        const auto legacyJson = createLegacyF0Json(data, 0);
        auto rootObj = legacyJson.toObject();
        auto contentObj = rootObj["content"].toObject();
        contentObj.remove("version");
        rootObj["content"] = contentObj;
        const auto corruptedJson = QJsonValue(rootObj);

        JsonTestStruct_v0 loaded;
        // This might trigger format error or version error depending on implementation
        EXPECT_THROW(ssJsonLoad(corruptedJson, loaded), std::runtime_error);
    }
}

TEST(SuitableStruct, LegacyFormatCompatibilityJson_DirectLegacyFormat)
{
    // Test legacy format that was saved directly without version wrapper
    // This simulates very old format: {"hash": X, "content": data}
    JsonTestStruct_v0 original;
    original.value = 123;

    QJsonObject data;
    data["value"] = original.value;
    const auto legacyJson = createLegacyDirectJson(data);

    JsonTestStruct_v0 loaded;
    ssJsonLoad(legacyJson, loaded);

    EXPECT_EQ(original, loaded);
}

TEST(SuitableStruct, LegacyFormatCompatibilityJson_MixedNewAndLegacyFormats)
{
    // Test that new format and legacy format can coexist
    JsonTestStruct_v1 original;
    original.value = 789;
    original.extra = 1.23f;

    // Save with new format
    const auto newFormatJson = ssJsonSave(original);
    JsonTestStruct_v1 loadedNew;
    ssJsonLoad(newFormatJson, loadedNew);
    EXPECT_EQ(original, loadedNew);

    // Create equivalent legacy format
    QJsonObject data;
    data["value"] = original.value;
    data["extra"] = static_cast<double>(original.extra);
    const auto legacyJson = createLegacyF0Json(data, 1);

    JsonTestStruct_v1 loadedLegacy;
    ssJsonLoad(legacyJson, loadedLegacy);
    EXPECT_EQ(original, loadedLegacy);

    // Both should be equivalent
    EXPECT_EQ(loadedNew, loadedLegacy);
}

TEST(SuitableStruct, LegacyFormatCompatibilityJson_LegacyFormatStructureValidation)
{
    // Validate that we can distinguish between new and legacy formats
    JsonTestStruct_v1 test;
    test.value = 100;
    test.extra = 2.5f;

    // New format should have ss_format_version
    const auto newJson = ssJsonSave(test, true);
    ASSERT_TRUE(newJson.isObject());
    const auto newObj = newJson.toObject();
    ASSERT_TRUE(newObj.contains("ss_format_version"));
    ASSERT_TRUE(newObj.contains("segments"));

    // Legacy format should NOT have ss_format_version
    QJsonObject data;
    data["value"] = test.value;
    data["extra"] = static_cast<double>(test.extra);
    const auto legacyJson = createLegacyF0Json(data, 1);

    ASSERT_TRUE(legacyJson.isObject());
    const auto legacyObj = legacyJson.toObject();
    ASSERT_FALSE(legacyObj.contains("ss_format_version"));
    ASSERT_FALSE(legacyObj.contains("segments"));
    ASSERT_TRUE(legacyObj.contains("hash"));
    ASSERT_TRUE(legacyObj.contains("content"));

    // Both should load to the same result
    JsonTestStruct_v1 fromNew, fromLegacy;
    ssJsonLoad(newJson, fromNew);
    ssJsonLoad(legacyJson, fromLegacy);
    EXPECT_EQ(fromNew, fromLegacy);
}

TEST(SuitableStruct, LegacyFormatCompatibilityJson_PrimitiveTypeLegacyFormat)
{
    // Test legacy format for primitive types (they should use KEY_CONTENT format)
    constexpr int originalValue = 42;

    // Legacy format for primitives: {"hash": X, "content": value}
    QJsonObject legacyRoot;
    legacyRoot["hash"] = static_cast<int>(Internal::ssJsonHashValue_F0(QJsonValue(originalValue)));
    legacyRoot["content"] = originalValue;

    const auto legacyJson = QJsonValue(legacyRoot);
    int loadedValue;
    ssJsonLoad(legacyJson, loadedValue);

    EXPECT_EQ(originalValue, loadedValue);
}

#endif // #ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
