/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <SuitableStruct/SerializerJson.h>
#include <SuitableStruct/Comparisons.h>

using namespace SuitableStruct;

namespace {

template<typename T>
static bool compareFloat(T a, T b) {
    return std::abs(a - b) <= std::numeric_limits<T>::epsilon();
}

// Plain structs without any serialization methods for JSON custom handlers
struct JsonStruct_v0
{
    int a{};
    float b{};

    bool operator==(const JsonStruct_v0& rhs) const {
        return a == rhs.a && compareFloat(b, rhs.b);
    }
};

struct JsonStruct_v1
{
    int a{};
    float b{};
    std::string c;

    bool operator==(const JsonStruct_v1& rhs) const {
        return a == rhs.a && compareFloat(b, rhs.b) && c == rhs.c;
    }
};

struct JsonStruct_v2
{
    int a{};
    float b{};
    std::string c;
    double d{};

    bool operator==(const JsonStruct_v2& rhs) const {
        return a == rhs.a &&
               compareFloat(b, rhs.b) &&
               c == rhs.c &&
               compareFloat(d, rhs.d);
    }
};

} // namespace

// External JSON handlers for version 0
namespace SuitableStruct {
template<>
struct Handlers<JsonStruct_v0> : public std::true_type
{
    static QJsonValue ssJsonSaveImpl(const JsonStruct_v0& v) {
        QJsonObject obj;
        obj["a"] = v.a;
        obj["b"] = static_cast<double>(v.b);
        return obj;
    }

    static void ssJsonLoadImpl(const QJsonValue& src, JsonStruct_v0& v) {
        const auto obj = src.toObject();
        v.a = obj["a"].toInt();
        v.b = static_cast<float>(obj["b"].toDouble());
    }
};
} // namespace SuitableStruct

// External JSON handlers for version 1
namespace SuitableStruct {
template<>
struct Handlers<JsonStruct_v1> : public std::true_type
{
    using ssVersions = std::tuple<JsonStruct_v0, JsonStruct_v1>;

    static QJsonValue ssJsonSaveImpl(const JsonStruct_v1& v) {
        QJsonObject obj;
        obj["a"] = v.a;
        obj["b"] = static_cast<double>(v.b);
        obj["c"] = QString::fromStdString(v.c);
        return obj;
    }

    static void ssJsonLoadImpl(const QJsonValue& src, JsonStruct_v1& v) {
        const auto obj = src.toObject();
        v.a = obj["a"].toInt();
        v.b = static_cast<float>(obj["b"].toDouble());
        v.c = obj["c"].toString().toStdString();
    }

    // Conversion methods in JSON handlers
    static void ssUpgradeFrom(const JsonStruct_v0& prev, JsonStruct_v1& current) {
        current.a = prev.a;
        current.b = prev.b;
        current.c = "default";
    }

    static void ssDowngradeTo(const JsonStruct_v1& current, JsonStruct_v0& downgraded) {
        downgraded.a = current.a;
        downgraded.b = current.b;
    }
};
} // namespace SuitableStruct

// External JSON handlers for version 2
namespace SuitableStruct {
template<>
struct Handlers<JsonStruct_v2> : public std::true_type
{
    using ssVersions = std::tuple<JsonStruct_v0, JsonStruct_v1, JsonStruct_v2>;

    static QJsonValue ssJsonSaveImpl(const JsonStruct_v2& v) {
        QJsonObject obj;
        obj["a"] = v.a;
        obj["b"] = static_cast<double>(v.b);
        obj["c"] = QString::fromStdString(v.c);
        obj["d"] = v.d;
        return obj;
    }

    static void ssJsonLoadImpl(const QJsonValue& src, JsonStruct_v2& v) {
        const auto obj = src.toObject();
        v.a = obj["a"].toInt();
        v.b = static_cast<float>(obj["b"].toDouble());
        v.c = obj["c"].toString().toStdString();
        v.d = obj["d"].toDouble();
    }

    // Conversion methods in JSON handlers
    static void ssUpgradeFrom(const JsonStruct_v1& prev, JsonStruct_v2& current) {
        current.a = prev.a;
        current.b = prev.b;
        current.c = prev.c;
        current.d = 0.0;
    }

    static void ssDowngradeTo(const JsonStruct_v2& current, JsonStruct_v1& downgraded) {
        downgraded.a = current.a;
        downgraded.b = current.b;
        downgraded.c = current.c;
    }
};
} // namespace SuitableStruct

TEST(SuitableStruct, JsonCustomHandlersVersioning_SameVersion)
{
    // Test saving and loading v0
    {
        JsonStruct_v0 initial, deserialized;
        initial.a = 42;
        initial.b = 3.14f;

        const auto json = ssJsonSave(initial);
        ssJsonLoad(json, deserialized);
        ASSERT_EQ(initial, deserialized);
    }

    // Test saving and loading v1
    {
        JsonStruct_v1 initial, deserialized;
        initial.a = 42;
        initial.b = 3.14f;
        initial.c = "test";

        const auto json = ssJsonSave(initial);
        ssJsonLoad(json, deserialized);
        ASSERT_EQ(initial, deserialized);
    }

    // Test saving and loading v2
    {
        JsonStruct_v2 initial, deserialized;
        initial.a = 42;
        initial.b = 3.14f;
        initial.c = "test";
        initial.d = 2.718;

        const auto json = ssJsonSave(initial);
        ssJsonLoad(json, deserialized);
        ASSERT_EQ(initial, deserialized);
    }
}

TEST(SuitableStruct, JsonCustomHandlersVersioning_MiddleVersion)
{
    // Test saving middle version (v1) and loading into new version
    {
        JsonStruct_v1 middle;
        middle.a = 42;
        middle.b = 3.14f;
        middle.c = "test";

        const auto json = ssJsonSave(middle);

        JsonStruct_v2 deserialized;
        ssJsonLoad(json, deserialized);

        ASSERT_EQ(deserialized.a, middle.a);
        ASSERT_FLOAT_EQ(deserialized.b, middle.b);
        ASSERT_EQ(deserialized.c, middle.c);
        ASSERT_DOUBLE_EQ(deserialized.d, 0.0);
    }

    // Test saving middle version (v1) and loading into old version
    {
        JsonStruct_v1 middle;
        middle.a = 42;
        middle.b = 3.14f;
        middle.c = "test";

        const auto json = ssJsonSave(middle);

        JsonStruct_v0 old;
        ssJsonLoad(json, old);

        ASSERT_EQ(old.a, middle.a);
        ASSERT_FLOAT_EQ(old.b, middle.b);
    }
}

TEST(SuitableStruct, JsonCustomHandlersVersioning_MultipleUpgrades)
{
    // Test multiple version upgrades in sequence
    JsonStruct_v0 v0;
    v0.a = 42;
    v0.b = 3.14f;

    const auto json = ssJsonSave(v0);

    // First upgrade to v1
    JsonStruct_v1 v1;
    ssJsonLoad(json, v1);
    ASSERT_EQ(v1.a, v0.a);
    ASSERT_FLOAT_EQ(v1.b, v0.b);
    ASSERT_EQ(v1.c, "default");

    // Then upgrade to v2
    JsonStruct_v2 v2;
    ssJsonLoad(json, v2);
    ASSERT_EQ(v2.a, v0.a);
    ASSERT_FLOAT_EQ(v2.b, v0.b);
    ASSERT_EQ(v2.c, "default");
    ASSERT_DOUBLE_EQ(v2.d, 0.0);
}

TEST(SuitableStruct, JsonCustomHandlersVersioning_MultipleDowngrades)
{
    // Test multiple version downgrades in sequence
    JsonStruct_v2 v2;
    v2.a = 42;
    v2.b = 3.14f;
    v2.c = "test";
    v2.d = 2.718;

    const auto json = ssJsonSave(v2);

    // First downgrade to v1
    JsonStruct_v1 v1;
    ssJsonLoad(json, v1);
    ASSERT_EQ(v1.a, v2.a);
    ASSERT_FLOAT_EQ(v1.b, v2.b);
    ASSERT_EQ(v1.c, v2.c);

    // Then downgrade to v0
    JsonStruct_v0 v0;
    ssJsonLoad(json, v0);
    ASSERT_EQ(v0.a, v2.a);
    ASSERT_FLOAT_EQ(v0.b, v2.b);
}

TEST(SuitableStruct, JsonCustomHandlersVersioning_NonProtectedMode)
{
    // Test that custom handlers work in non-protected mode too
    JsonStruct_v2 original;
    original.a = 123;
    original.b = 4.56f;
    original.c = "non_protected";
    original.d = 7.89;

    const auto json = ssJsonSave(original, false); // Non-protected mode
    JsonStruct_v2 loaded;
    ssJsonLoad(json, loaded, SSLoadMode::NonProtectedDefault);

    ASSERT_EQ(original, loaded);
}

TEST(SuitableStruct, JsonCustomHandlersVersioning_CrossVersionNonProtected)
{
    // Test cross-version loading in non-protected mode with custom handlers
    JsonStruct_v1 original;
    original.a = 789;
    original.b = 1.23f;
    original.c = "cross_version";

    const auto json = ssJsonSave(original, false); // Save as v1, non-protected
    JsonStruct_v2 loaded;                     // Load as v2
    ssJsonLoad(json, loaded, SSLoadMode::NonProtectedDefault);

    ASSERT_EQ(loaded.a, 789);
    ASSERT_FLOAT_EQ(loaded.b, 1.23f);
    ASSERT_EQ(loaded.c, "cross_version");
    ASSERT_DOUBLE_EQ(loaded.d, 0.0); // Default from upgrade
}

TEST(SuitableStruct, JsonCustomHandlersVersioning_StructureValidation)
{
    // Test that the JSON structure is as expected for new format
    JsonStruct_v1 test;
    test.a = 100;
    test.b = 2.5f;
    test.c = "structure_test";

    const auto json = ssJsonSave(test, true); // Protected mode
    ASSERT_TRUE(json.isObject());

    const auto rootObj = json.toObject();
    ASSERT_TRUE(rootObj.contains("ss_format_version"));
    ASSERT_TRUE(rootObj.contains("hash"));
    ASSERT_TRUE(rootObj.contains("segments"));

    ASSERT_EQ(rootObj["ss_format_version"].toString(), "1.0");
    ASSERT_TRUE(rootObj["segments"].isArray());

    const auto segments = rootObj["segments"].toArray();
    ASSERT_EQ(segments.size(), 2); // Should have 2 segments (v0 and v1)

    // Check first segment (should be v1, the highest/current version)
    const auto firstSegment = segments[0].toObject();
    ASSERT_TRUE(firstSegment.contains("version_index"));
    ASSERT_TRUE(firstSegment.contains("data"));
    ASSERT_EQ(firstSegment["version_index"].toInt(), 1); // v1 has index 1

    // Check that the data contains our values
    const auto data = firstSegment["data"].toObject();
    ASSERT_EQ(data["a"].toInt(), 100);
    ASSERT_FLOAT_EQ(static_cast<float>(data["b"].toDouble()), 2.5f);
    ASSERT_EQ(data["c"].toString().toStdString(), "structure_test");
}

#endif // #ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
