/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <SuitableStruct/SerializerJson.h>
#include <SuitableStruct/Comparisons.h>

using namespace SuitableStruct;

namespace {

// Inner struct versioning for JSON
struct JsonInnerStruct_v0
{
    int value {};

    using ssVersions = std::tuple<JsonInnerStruct_v0>;
    auto ssTuple() const { return std::tie(value); }
    auto ssNamesTuple() const { return std::tie("value"); }
    SS_COMPARISONS_MEMBER(JsonInnerStruct_v0)
};

struct JsonInnerStruct_v1
{
    int value {};
    float extra {};

    using ssVersions = std::tuple<JsonInnerStruct_v0, JsonInnerStruct_v1>;
    auto ssTuple() const { return std::tie(value, extra); }
    auto ssNamesTuple() const { return std::tie("value", "extra"); }

    void ssUpgradeFrom(const JsonInnerStruct_v0& prev) {
        value = prev.value;
        extra = 1.5f;
    }

    void ssDowngradeTo(JsonInnerStruct_v0& next) const {
        next.value = value;
    }

    SS_COMPARISONS_MEMBER(JsonInnerStruct_v1)
};

struct JsonInnerStruct_v2
{
    int value {};
    double extra {};
    std::string tag;

    using ssVersions = std::tuple<JsonInnerStruct_v0, JsonInnerStruct_v1, JsonInnerStruct_v2>;
    auto ssTuple() const { return std::tie(value, extra, tag); }
    auto ssNamesTuple() const { return std::tie("value", "extra", "tag"); }

    void ssUpgradeFrom(const JsonInnerStruct_v1& prev) {
        value = prev.value;
        extra = static_cast<double>(prev.extra);
        tag = "upgraded";
    }

    void ssDowngradeTo(JsonInnerStruct_v1& next) const {
        next.value = value;
        next.extra = static_cast<float>(extra);
    }

    SS_COMPARISONS_MEMBER(JsonInnerStruct_v2)
};

// Outer struct versioning for JSON
struct JsonOuterStruct_v0
{
    JsonInnerStruct_v0 inner;
    std::string name;

    auto ssTuple() const { return std::tie(inner, name); }
    auto ssNamesTuple() const { return std::tie("inner", "name"); }
    SS_COMPARISONS_MEMBER(JsonOuterStruct_v0)
};

struct JsonOuterStruct_v1
{
    JsonInnerStruct_v1 inner;  // Uses newer version of inner struct
    std::string name;

    auto ssTuple() const { return std::tie(inner, name); }
    auto ssNamesTuple() const { return std::tie("inner", "name"); }
    SS_COMPARISONS_MEMBER(JsonOuterStruct_v1)
};

struct JsonOuterStruct_v2
{
    JsonInnerStruct_v2 inner;  // Uses even newer version of inner struct
    std::string name;

    auto ssTuple() const { return std::tie(inner, name); }
    auto ssNamesTuple() const { return std::tie("inner", "name"); }
    SS_COMPARISONS_MEMBER(JsonOuterStruct_v2)
};

} // namespace

TEST(SuitableStruct, JsonNestedVersioning_SaveLoadOuterStructV0)
{
    JsonOuterStruct_v0 original;
    original.inner.value = 42;
    original.name = "test_v0";

    const auto saved = ssJsonSave(original);
    JsonOuterStruct_v0 loaded;
    ssJsonLoad(saved, loaded);

    EXPECT_EQ(original, loaded);
}

TEST(SuitableStruct, JsonNestedVersioning_SaveLoadOuterStructV1)
{
    JsonOuterStruct_v1 original;
    original.inner.value = 42;
    original.inner.extra = 3.14f;
    original.name = "test_v1";

    const auto saved = ssJsonSave(original);
    JsonOuterStruct_v1 loaded;
    ssJsonLoad(saved, loaded);

    EXPECT_EQ(original, loaded);
}

TEST(SuitableStruct, JsonNestedVersioning_SaveLoadOuterStructV2)
{
    JsonOuterStruct_v2 original;
    original.inner.value = 42;
    original.inner.extra = 3.14;
    original.inner.tag = "test_tag";
    original.name = "test_v2";

    const auto saved = ssJsonSave(original);
    JsonOuterStruct_v2 loaded;
    ssJsonLoad(saved, loaded);

    EXPECT_EQ(original, loaded);
}

TEST(SuitableStruct, JsonNestedVersioning_UpgradeOuterStructV0ToV1)
{
    JsonOuterStruct_v0 original;
    original.inner.value = 42;
    original.name = "test_upgrade";

    const auto saved = ssJsonSave(original);
    JsonOuterStruct_v1 loaded;
    ssJsonLoad(saved, loaded);

    EXPECT_EQ(loaded.inner.value, 42);
    EXPECT_FLOAT_EQ(loaded.inner.extra, 1.5f);  // Default value from upgrade
    EXPECT_EQ(loaded.name, "test_upgrade");
}

TEST(SuitableStruct, JsonNestedVersioning_UpgradeOuterStructV1ToV2)
{
    JsonOuterStruct_v1 original;
    original.inner.value = 42;
    original.inner.extra = 3.14f;
    original.name = "test_upgrade";

    const auto saved = ssJsonSave(original);
    JsonOuterStruct_v2 loaded;
    ssJsonLoad(saved, loaded);

    EXPECT_EQ(loaded.inner.value, 42);
    EXPECT_FLOAT_EQ(loaded.inner.extra, 3.14f);  // Converted from float to double
    EXPECT_EQ(loaded.inner.tag, "upgraded");     // Default value from upgrade
    EXPECT_EQ(loaded.name, "test_upgrade");
}

TEST(SuitableStruct, JsonNestedVersioning_DowngradeOuterStructV2ToV1)
{
    JsonOuterStruct_v2 original;
    original.inner.value = 42;
    original.inner.extra = 3.14;
    original.inner.tag = "test_tag";
    original.name = "test_downgrade";

    const auto saved = ssJsonSave(original);
    JsonOuterStruct_v1 loaded;
    ssJsonLoad(saved, loaded);

    EXPECT_EQ(loaded.inner.value, 42);
    EXPECT_FLOAT_EQ(loaded.inner.extra, 3.14f);  // Converted from double to float
    EXPECT_EQ(loaded.name, "test_downgrade");
}

TEST(SuitableStruct, JsonNestedVersioning_DowngradeOuterStructV1ToV0)
{
    JsonOuterStruct_v1 original;
    original.inner.value = 42;
    original.inner.extra = 3.14f;
    original.name = "test_downgrade";

    const auto saved = ssJsonSave(original);
    JsonOuterStruct_v0 loaded;
    ssJsonLoad(saved, loaded);

    EXPECT_EQ(loaded.inner.value, 42);
    EXPECT_EQ(loaded.name, "test_downgrade");
}

TEST(SuitableStruct, JsonNestedVersioning_UpgradeOuterStructV0ToV2)
{
    JsonOuterStruct_v0 original;
    original.inner.value = 42;
    original.name = "test_upgrade";

    const auto saved = ssJsonSave(original);
    JsonOuterStruct_v2 loaded;
    ssJsonLoad(saved, loaded);

    EXPECT_EQ(loaded.inner.value, 42);
    EXPECT_FLOAT_EQ(loaded.inner.extra, 1.5f);  // Default value from upgrade
    EXPECT_EQ(loaded.inner.tag, "upgraded");    // Default value from upgrade
    EXPECT_EQ(loaded.name, "test_upgrade");
}

TEST(SuitableStruct, JsonNestedVersioning_DowngradeOuterStructV2ToV0)
{
    JsonOuterStruct_v2 original;
    original.inner.value = 42;
    original.inner.extra = 3.14;
    original.inner.tag = "test_tag";
    original.name = "test_downgrade";

    const auto saved = ssJsonSave(original);
    JsonOuterStruct_v0 loaded;
    ssJsonLoad(saved, loaded);

    EXPECT_EQ(loaded.inner.value, 42);
    EXPECT_EQ(loaded.name, "test_downgrade");
}

TEST(SuitableStruct, JsonNestedVersioning_NonProtectedModeConsistency)
{
    // Test that nested versioning works the same in non-protected mode
    JsonOuterStruct_v2 original;
    original.inner.value = 123;
    original.inner.extra = 4.56;
    original.inner.tag = "non_protected";
    original.name = "test_non_protected";

    const auto saved = ssJsonSave(original, false); // Non-protected mode
    JsonOuterStruct_v2 loaded;
    ssJsonLoad(saved, loaded, SSLoadMode::NonProtectedDefault);

    EXPECT_EQ(original, loaded);
}

TEST(SuitableStruct, JsonNestedVersioning_CrossVersionNonProtectedMode)
{
    // Test cross-version loading in non-protected mode
    JsonOuterStruct_v1 original;
    original.inner.value = 789;
    original.inner.extra = 7.89f;
    original.name = "cross_version";

    const auto saved = ssJsonSave(original, false); // Save as v1, non-protected
    JsonOuterStruct_v2 loaded;                // Load as v2
    ssJsonLoad(saved, loaded, SSLoadMode::NonProtectedDefault);

    EXPECT_EQ(loaded.inner.value, 789);
    EXPECT_FLOAT_EQ(loaded.inner.extra, 7.89f);
    EXPECT_EQ(loaded.inner.tag, "upgraded");  // Default from upgrade
    EXPECT_EQ(loaded.name, "cross_version");
}

#endif // #ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
