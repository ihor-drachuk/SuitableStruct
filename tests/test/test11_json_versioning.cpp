/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/SerializerJson.h>
#include <SuitableStruct/Containers/vector.h>
#include <SuitableStruct/Exceptions.h>
#include <QJsonObject>
#include <QJsonValue>
#include <string>

using namespace SuitableStruct;

namespace {

struct Struct1
{
    int a{}, b{};

    auto ssTuple() const { return std::tie(a, b); }
    auto ssNamesTuple() const { return std::tie("a", "b"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct1)
};

struct Struct2
{
    std::vector<int> values;

    using ssVersions = std::tuple<Struct1, Struct2>;
    auto ssTuple() const { return std::tie(values); }
    auto ssNamesTuple() const { return std::tie("values"); }
    void ssUpgradeFrom(const Struct1& prev) { values = {prev.a, prev.b}; }
    void ssDowngradeTo(Struct1& next) const {
        if (!values.empty()) {
            next.a = values[0];
            next.b = values.size() > 1 ? values[1] : 0;
        }
    }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct2)
};

struct Struct3
{
    std::vector<std::string> values;

    using ssVersions = std::tuple<Struct1, Struct2, Struct3>;
    auto ssTuple() const { return std::tie(values); }
    auto ssNamesTuple() const { return std::tie("values"); }

    void ssUpgradeFrom(const Struct2& prev) {
        values.clear();

        for (const auto& x : prev.values)
            values.push_back(std::to_string(x));
    }

    void ssDowngradeTo(Struct2& next) const {
        next.values.clear();
        for (const auto& x : values) {
            try {
                next.values.push_back(std::stoi(x));
            } catch (...) {
                next.values.push_back(0);
            }
        }
    }

    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct3)
};

using Struct = Struct3; // Latest version

struct NestedStructs
{
    Struct value;

    auto ssTuple() const { return std::tie(value); }
    auto ssNamesTuple() const { return std::tie("value"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(NestedStructs)
};

struct NestedStructs_Old_Helper_For_Test
{
    Struct1 value;

    auto ssTuple() const { return std::tie(value); }
    auto ssNamesTuple() const { return std::tie("value"); }
};

struct JsonCastableStruct_v0
{
    int value {};

    auto ssTuple() const { return std::tie(value); }
    auto ssNamesTuple() const { return std::tie("value"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(JsonCastableStruct_v0)
};

struct JsonCastableStruct_v1
{
    int value {};
    std::string extra;

    using ssVersions = std::tuple<JsonCastableStruct_v0, JsonCastableStruct_v1>;
    auto ssTuple() const { return std::tie(value, extra); }
    auto ssNamesTuple() const { return std::tie("value", "extra"); }

    // Explicit conversion constructor for C++ casting fallback
    JsonCastableStruct_v1(const JsonCastableStruct_v0& old) : value(old.value), extra("default") {}
    JsonCastableStruct_v1() = default;

    SS_COMPARISONS_MEMBER_ONLY_EQ(JsonCastableStruct_v1)
};

} // namespace

TEST(SuitableStruct, JsonVersioningTest_1_to_2)
{
    Struct1 s1;
    s1.a = 17;
    s1.b = 42;

    const auto saved = ssJsonSave(s1);

    Struct2 s2;
    ssJsonLoad(saved, s2);

    ASSERT_EQ(s2.values.size(), 2);
    ASSERT_EQ(s2.values[0], 17);
    ASSERT_EQ(s2.values[1], 42);
}

TEST(SuitableStruct, JsonVersioningTest_2_to_3)
{
    Struct2 s2;
    s2.values = {17, 42};

    const auto saved = ssJsonSave(s2);

    Struct3 s3;
    ssJsonLoad(saved, s3);

    ASSERT_EQ(s3.values.size(), 2);
    ASSERT_EQ(s3.values[0], "17");
    ASSERT_EQ(s3.values[1], "42");
}

TEST(SuitableStruct, JsonVersioningTest_1_to_3)
{
    Struct1 s1;
    s1.a = 17;
    s1.b = 42;

    const auto saved = ssJsonSave(s1);

    Struct3 s3;
    ssJsonLoad(saved, s3);

    ASSERT_EQ(s3.values.size(), 2);
    ASSERT_EQ(s3.values[0], "17");
    ASSERT_EQ(s3.values[1], "42");
}

TEST(SuitableStruct, JsonVersioningTest_1_to_1)
{
    Struct1 s1;
    s1.a = 17;
    s1.b = 42;

    const auto saved = ssJsonSave(s1);

    Struct1 s1r;
    ssJsonLoad(saved, s1r);

    ASSERT_EQ(s1r, s1);
}

TEST(SuitableStruct, JsonVersioningTest_2_to_2)
{
    Struct2 s2;
    s2.values = {17, 42, 99};

    const auto saved = ssJsonSave(s2);

    Struct2 s2r;
    ssJsonLoad(saved, s2r);

    ASSERT_EQ(s2r, s2);
}

TEST(SuitableStruct, JsonVersioningTest_3_to_3)
{
    Struct3 s3;
    s3.values = {"17", "42", "hello"};

    const auto saved = ssJsonSave(s3);

    Struct3 s3r;
    ssJsonLoad(saved, s3r);

    ASSERT_EQ(s3r, s3);
}

TEST(SuitableStruct, JsonVersioningTest_3_to_2)
{
    Struct3 s3;
    s3.values = {"17", "42", "99"};

    const auto saved = ssJsonSave(s3);

    Struct2 s2;
    ssJsonLoad(saved, s2);

    ASSERT_EQ(s2.values.size(), 3);
    ASSERT_EQ(s2.values[0], 17);
    ASSERT_EQ(s2.values[1], 42);
    ASSERT_EQ(s2.values[2], 99);
}

TEST(SuitableStruct, JsonVersioningTest_3_to_1)
{
    Struct3 s3;
    s3.values = {"17", "42", "99"};

    const auto saved = ssJsonSave(s3);

    Struct1 s1;
    ssJsonLoad(saved, s1);

    ASSERT_EQ(s1.a, 17);
    ASSERT_EQ(s1.b, 42);
}

TEST(SuitableStruct, JsonVersioningTest_2_to_1)
{
    Struct2 s2;
    s2.values = {17, 42, 99};

    const auto saved = ssJsonSave(s2);

    Struct1 s1;
    ssJsonLoad(saved, s1);

    ASSERT_EQ(s1.a, 17);
    ASSERT_EQ(s1.b, 42);
}

TEST(SuitableStruct, JsonVersioningTest_2_to_1_EmptyValues)
{
    Struct2 s2;
    s2.values = {};

    const auto saved = ssJsonSave(s2);

    Struct1 s1;
    ssJsonLoad(saved, s1);

    ASSERT_EQ(s1.a, 0);
    ASSERT_EQ(s1.b, 0);
}

TEST(SuitableStruct, JsonVersioningTest_2_to_1_SingleValue)
{
    Struct2 s2;
    s2.values = {17};

    const auto saved = ssJsonSave(s2);

    Struct1 s1;
    ssJsonLoad(saved, s1);

    ASSERT_EQ(s1.a, 17);
    ASSERT_EQ(s1.b, 0);
}

TEST(SuitableStruct, JsonVersioningTest_3_to_2_InvalidNumbers)
{
    Struct3 s3;
    s3.values = {"17", "not_a_number", "42"};

    const auto saved = ssJsonSave(s3);

    Struct2 s2;
    ssJsonLoad(saved, s2);

    ASSERT_EQ(s2.values.size(), 3);
    ASSERT_EQ(s2.values[0], 17);
    ASSERT_EQ(s2.values[1], 0);  // Invalid number becomes 0
    ASSERT_EQ(s2.values[2], 42);
}

TEST(SuitableStruct, JsonVersioningNested)
{
    NestedStructs_Old_Helper_For_Test value1;
    value1.value = {1, 2};

    const auto saved = ssJsonSave(value1);

    NestedStructs value2;
    ssJsonLoad(saved, value2);

    NestedStructs value2ref;
    value2ref.value.values = {"1", "2"};

    ASSERT_EQ(value2, value2ref);
}

TEST(SuitableStruct, JsonVersioningNested_3_to_1)
{
    NestedStructs value;
    value.value.values = {"17", "42", "hello"};

    const auto saved = ssJsonSave(value);

    NestedStructs_Old_Helper_For_Test value1;
    ssJsonLoad(saved, value1);

    ASSERT_EQ(value1.value.a, 17);
    ASSERT_EQ(value1.value.b, 42);
}

TEST(SuitableStruct, JsonVersioningUpgradeCastingFallback)
{
    // Test that C++ casting fallback works for JSON upgrades when ssUpgradeFrom is not defined
    JsonCastableStruct_v0 oldStruct;
    oldStruct.value = 123;

    const auto saved = ssJsonSave(oldStruct);

    JsonCastableStruct_v1 newStruct;
    ssJsonLoad(saved, newStruct);

    // Should have converted using the C++ constructor
    ASSERT_EQ(newStruct.value, 123);
    ASSERT_EQ(newStruct.extra, "default");
}

TEST(SuitableStruct, JsonVersioningTest_InvalidJson)
{
    QJsonObject invalidStructure;
    invalidStructure["unknown_field"] = "invalid";

    Struct1 s1;
    EXPECT_THROW(ssJsonLoad(QJsonValue(invalidStructure), s1), std::exception);

    Struct2 s2;
    EXPECT_THROW(ssJsonLoad(QJsonValue(invalidStructure), s2), std::exception);

    Struct3 s3;
    EXPECT_THROW(ssJsonLoad(QJsonValue(invalidStructure), s3), std::exception);
}

TEST(SuitableStruct, JsonVersioningTest_EmptyJson)
{
    QJsonObject emptyJson;

    Struct1 s1;
    EXPECT_THROW(ssJsonLoad(QJsonValue(emptyJson), s1), std::exception);

    Struct2 s2;
    EXPECT_THROW(ssJsonLoad(QJsonValue(emptyJson), s2), std::exception);

    Struct3 s3;
    EXPECT_THROW(ssJsonLoad(QJsonValue(emptyJson), s3), std::exception);
}

TEST(SuitableStruct, JsonVersioning)
{
    Struct1 s1;
    s1 = {1,2};
    const auto saved1 = ssJsonSave(s1);

    Struct2 s2, s2ref;
    ssJsonLoad(saved1, s2);
    s2ref.values = {1,2};
    ASSERT_EQ(s2, s2ref);
    const auto saved2 = ssJsonSave(s2);

    Struct3 s3, s3ref;
    ssJsonLoad(saved2, s3);
    s3ref.values = {"1","2"};
    ASSERT_EQ(s3, s3ref);
}

#endif // #ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
