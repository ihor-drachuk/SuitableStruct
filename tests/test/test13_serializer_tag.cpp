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

struct NonDefaultConstructible {
    int value;

    // Constructor taking serializer tag
    explicit NonDefaultConstructible(SS_SERIALIZER_TAG) : value(0) {}

    // Regular constructor
    explicit NonDefaultConstructible(int v) : value(v) {}

    auto ssTuple() const { return std::tie(value); }
    auto ssNamesTuple() const { return std::tie("value"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(NonDefaultConstructible);
};

struct DefaultConstructible {
    int value{};

    auto ssTuple() const { return std::tie(value); }
    auto ssNamesTuple() const { return std::tie("value"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(DefaultConstructible);
};

} // namespace

TEST(SuitableStruct, SerializerTag)
{
    // Test non-default constructible type
    NonDefaultConstructible initial(42), deserialized(0);
    initial.value = 100;

    auto buffer = ssSave(initial);
    ASSERT_NE(initial, deserialized);
    ssLoad(buffer, deserialized);
    ASSERT_EQ(initial, deserialized);

    // Test default constructible type
    DefaultConstructible initial2, deserialized2;
    initial2.value = 200;

    buffer = ssSave(initial2);
    ASSERT_NE(initial2, deserialized2);
    ssLoad(buffer, deserialized2);
    ASSERT_EQ(initial2, deserialized2);
}

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
TEST(SuitableStruct, SerializerTag_Json)
{
    // Test non-default constructible type
    NonDefaultConstructible initial(42), deserialized(0);
    initial.value = 100;

    auto json = ssJsonSave(initial);
    ASSERT_NE(initial, deserialized);
    SuitableStruct::ssJsonLoad(json, deserialized);
    ASSERT_EQ(initial, deserialized);

    // Test default constructible type
    DefaultConstructible initial2, deserialized2;
    initial2.value = 200;

    json = ssJsonSave(initial2);
    ASSERT_NE(initial2, deserialized2);
    SuitableStruct::ssJsonLoad(json, deserialized2);
    ASSERT_EQ(initial2, deserialized2);
}
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

TEST(SuitableStruct, SerializerTag_SmartPointers)
{
    // Test shared_ptr
    auto initial = std::make_shared<NonDefaultConstructible>(42);
    initial->value = 100;
    std::shared_ptr<NonDefaultConstructible> deserialized;

    auto buffer = ssSave(initial);
    ASSERT_NE(initial, deserialized);
    ssLoad(buffer, deserialized);
    ASSERT_NE(nullptr, deserialized);
    ASSERT_EQ(initial->value, deserialized->value);

    // Test unique_ptr
    auto initial2 = std::make_unique<DefaultConstructible>();
    initial2->value = 200;
    std::unique_ptr<DefaultConstructible> deserialized2;

    buffer = ssSave(initial2);
    ASSERT_NE(initial2, deserialized2);
    ssLoad(buffer, deserialized2);
    ASSERT_NE(nullptr, deserialized2);
    ASSERT_EQ(initial2->value, deserialized2->value);
}

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
TEST(SuitableStruct, SerializerTag_SmartPointers_Json)
{
    // Test shared_ptr
    auto initial = std::make_shared<NonDefaultConstructible>(42);
    initial->value = 100;
    std::shared_ptr<NonDefaultConstructible> deserialized;

    auto json = ssJsonSave(initial);
    ASSERT_NE(initial, deserialized);
    ssJsonLoad(json, deserialized);
    ASSERT_NE(nullptr, deserialized);
    ASSERT_EQ(initial->value, deserialized->value);

    // Test unique_ptr
    auto initial2 = std::make_unique<DefaultConstructible>();
    initial2->value = 200;
    std::unique_ptr<DefaultConstructible> deserialized2;

    json = ssJsonSave(initial2);
    ASSERT_NE(initial2, deserialized2);
    ssJsonLoad(json, deserialized2);
    ASSERT_NE(nullptr, deserialized2);
    ASSERT_EQ(initial2->value, deserialized2->value);
}
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
