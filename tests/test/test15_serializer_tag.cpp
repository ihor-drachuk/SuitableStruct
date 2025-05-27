/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <SuitableStruct/SerializerJson.h>
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

using namespace SuitableStruct;

namespace {

struct NestedDefaultConstructible {
    int nestedValue{42};

    auto ssTuple() const { return std::tie(nestedValue); }
    auto ssNamesTuple() const { return std::tie("nestedValue"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(NestedDefaultConstructible)
};

struct NestedNonDefaultConstructible {
    int nestedValue;

    explicit NestedNonDefaultConstructible(SS_SERIALIZER_TAG) : nestedValue(0) {}
    explicit NestedNonDefaultConstructible(int v) : nestedValue(v) {}

    auto ssTuple() const { return std::tie(nestedValue); }
    auto ssNamesTuple() const { return std::tie("nestedValue"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(NestedNonDefaultConstructible)
};

struct NonDefaultConstructible {
    int value;
    NestedDefaultConstructible nestedDefault;
    NestedNonDefaultConstructible nestedNonDefault;

    // Constructor taking serializer tag
    explicit NonDefaultConstructible(SS_SERIALIZER_TAG) : value(0), nestedNonDefault(SS_SERIALIZER_TAG{}) {}

    // Regular constructor
    explicit NonDefaultConstructible(int v) : value(v), nestedNonDefault(v * 2) {}

    auto ssTuple() const { return std::tie(value, nestedDefault, nestedNonDefault); }
    auto ssNamesTuple() const { return std::tie("value", "nestedDefault", "nestedNonDefault"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(NonDefaultConstructible)
};

struct DefaultConstructible {
    int value{};
    NestedDefaultConstructible nestedDefault;
    NestedNonDefaultConstructible nestedNonDefault{SS_SERIALIZER_TAG{}};

    auto ssTuple() const { return std::tie(value, nestedDefault, nestedNonDefault); }
    auto ssNamesTuple() const { return std::tie("value", "nestedDefault", "nestedNonDefault"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(DefaultConstructible)
};

} // namespace

TEST(SuitableStruct, SerializerTag)
{
    // Test non-default constructible type
    NonDefaultConstructible initial(42), deserialized(0);
    initial.value = 100;
    initial.nestedDefault.nestedValue = 150;
    initial.nestedNonDefault.nestedValue = 200;

    const auto buffer = ssSave(initial);
    ASSERT_NE(initial, deserialized);
    ssLoad(buffer, deserialized);
    ASSERT_EQ(initial, deserialized);

    // Test default constructible type
    DefaultConstructible initial2, deserialized2;
    initial2.value = 200;
    initial2.nestedDefault.nestedValue = 250;
    initial2.nestedNonDefault.nestedValue = 300;

    const auto buffer2 = ssSave(initial2);
    ASSERT_NE(initial2, deserialized2);
    ssLoad(buffer2, deserialized2);
    ASSERT_EQ(initial2, deserialized2);
}

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
TEST(SuitableStruct, SerializerTag_Json)
{
    // Test non-default constructible type
    NonDefaultConstructible initial(42), deserialized(0);
    initial.value = 100;
    initial.nestedDefault.nestedValue = 150;
    initial.nestedNonDefault.nestedValue = 200;

    const auto json = ssJsonSave(initial);
    ASSERT_NE(initial, deserialized);
    SuitableStruct::ssJsonLoad(json, deserialized);
    ASSERT_EQ(initial, deserialized);

    // Test default constructible type
    DefaultConstructible initial2, deserialized2;
    initial2.value = 200;
    initial2.nestedDefault.nestedValue = 250;
    initial2.nestedNonDefault.nestedValue = 300;

    const auto json2 = ssJsonSave(initial2);
    ASSERT_NE(initial2, deserialized2);
    SuitableStruct::ssJsonLoad(json2, deserialized2);
    ASSERT_EQ(initial2, deserialized2);
}
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

TEST(SuitableStruct, SerializerTag_SmartPointers)
{
    // Test shared_ptr
    auto initial = std::make_shared<NonDefaultConstructible>(42);
    initial->value = 100;
    initial->nestedDefault.nestedValue = 150;
    initial->nestedNonDefault.nestedValue = 200;
    std::shared_ptr<NonDefaultConstructible> deserialized;

    const auto buffer = ssSave(initial);
    ASSERT_NE(initial, deserialized);
    ssLoad(buffer, deserialized);
    ASSERT_NE(nullptr, deserialized);
    ASSERT_EQ(initial->value, deserialized->value);
    ASSERT_EQ(initial->nestedDefault.nestedValue, deserialized->nestedDefault.nestedValue);
    ASSERT_EQ(initial->nestedNonDefault.nestedValue, deserialized->nestedNonDefault.nestedValue);

    // Test unique_ptr
    auto initial2 = std::make_unique<DefaultConstructible>();
    initial2->value = 200;
    initial2->nestedDefault.nestedValue = 250;
    initial2->nestedNonDefault.nestedValue = 300;
    std::unique_ptr<DefaultConstructible> deserialized2;

    const auto buffer2 = ssSave(initial2);
    ASSERT_NE(initial2, deserialized2);
    ssLoad(buffer2, deserialized2);
    ASSERT_NE(nullptr, deserialized2);
    ASSERT_EQ(initial2->value, deserialized2->value);
    ASSERT_EQ(initial2->nestedDefault.nestedValue, deserialized2->nestedDefault.nestedValue);
    ASSERT_EQ(initial2->nestedNonDefault.nestedValue, deserialized2->nestedNonDefault.nestedValue);
}

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
TEST(SuitableStruct, SerializerTag_SmartPointers_Json)
{
    // Test shared_ptr
    auto initial = std::make_shared<NonDefaultConstructible>(42);
    initial->value = 100;
    initial->nestedDefault.nestedValue = 150;
    initial->nestedNonDefault.nestedValue = 200;
    std::shared_ptr<NonDefaultConstructible> deserialized;

    const auto json = ssJsonSave(initial);
    ASSERT_NE(initial, deserialized);
    ssJsonLoad(json, deserialized);
    ASSERT_NE(nullptr, deserialized);
    ASSERT_EQ(initial->value, deserialized->value);
    ASSERT_EQ(initial->nestedDefault.nestedValue, deserialized->nestedDefault.nestedValue);
    ASSERT_EQ(initial->nestedNonDefault.nestedValue, deserialized->nestedNonDefault.nestedValue);

    // Test unique_ptr
    auto initial2 = std::make_unique<DefaultConstructible>();
    initial2->value = 200;
    initial2->nestedDefault.nestedValue = 250;
    initial2->nestedNonDefault.nestedValue = 300;
    std::unique_ptr<DefaultConstructible> deserialized2;

    const auto json2 = ssJsonSave(initial2);
    ASSERT_NE(initial2, deserialized2);
    ssJsonLoad(json2, deserialized2);
    ASSERT_NE(nullptr, deserialized2);
    ASSERT_EQ(initial2->value, deserialized2->value);
    ASSERT_EQ(initial2->nestedDefault.nestedValue, deserialized2->nestedDefault.nestedValue);
    ASSERT_EQ(initial2->nestedNonDefault.nestedValue, deserialized2->nestedNonDefault.nestedValue);
}
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
