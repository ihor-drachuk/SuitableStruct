/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>

#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/SerializerJson.h>

namespace {

struct Struct1
{
    int a{};
    mutable bool beforeSaveCalled{};
    mutable bool afterSaveCalled{};
    bool beforeLoadCalled{};
    bool afterLoadCalled{};

    auto ssTuple() const { return std::tie(a); }
    auto ssNamesTuple() const { return std::tie("a"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct1)

    SS_DEFINE_BEFORE_SAVE_CONST() { beforeSaveCalled = true; }
    SS_DEFINE_AFTER_SAVE_CONST() { afterSaveCalled = true; }
    SS_DEFINE_BEFORE_LOAD() { beforeLoadCalled = true; }
    SS_DEFINE_AFTER_LOAD() { afterLoadCalled = true; }
};

struct Struct2
{
    Struct1 s1{};
    mutable bool beforeSaveCalled{};
    mutable bool afterSaveCalled{};
    bool beforeLoadCalled{};
    bool afterLoadCalled{};

    auto ssTuple() const { return std::tie(s1); }
    auto ssNamesTuple() const { return std::tie("s1"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct2)

    SS_DEFINE_BEFORE_SAVE_CONST() { beforeSaveCalled = true; }
    SS_DEFINE_AFTER_SAVE_CONST() { afterSaveCalled = true; }
    SS_DEFINE_BEFORE_LOAD() { beforeLoadCalled = true; }
    SS_DEFINE_AFTER_LOAD() { afterLoadCalled = true; }
};

struct Struct3
{
    int a{};
    bool afterLoadCalled{};

    auto ssTuple() const { return std::tie(a); }
    auto ssNamesTuple() const { return std::tie("a"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct3)

    SS_DEFINE_AFTER_LOAD() { afterLoadCalled = true; }
};

} // namespace

TEST(SuitableStruct, PrePostOperations_Binary)
{
    Struct1 initial, deserialized;
    initial.a = 100;

    // Test save hooks
    auto buffer = SuitableStruct::ssSave(initial);
    ASSERT_TRUE(initial.beforeSaveCalled);
    ASSERT_TRUE(initial.afterSaveCalled);

    // Test load hooks
    ssLoad(buffer, deserialized);
    ASSERT_TRUE(deserialized.beforeLoadCalled);
    ASSERT_TRUE(deserialized.afterLoadCalled);
    ASSERT_EQ(deserialized.a, initial.a);
}

TEST(SuitableStruct, PrePostOperations_Binary_Nested)
{
    Struct2 initial, deserialized;
    initial.s1.a = 100;

    // Test save hooks
    auto buffer = SuitableStruct::ssSave(initial);
    ASSERT_TRUE(initial.beforeSaveCalled);
    ASSERT_TRUE(initial.afterSaveCalled);
    ASSERT_TRUE(initial.s1.beforeSaveCalled);
    ASSERT_TRUE(initial.s1.afterSaveCalled);

    // Test load hooks
    ssLoad(buffer, deserialized);
    ASSERT_TRUE(deserialized.beforeLoadCalled);
    ASSERT_TRUE(deserialized.afterLoadCalled);
    ASSERT_TRUE(deserialized.s1.beforeLoadCalled);
    ASSERT_TRUE(deserialized.s1.afterLoadCalled);
    ASSERT_EQ(deserialized.s1.a, initial.s1.a);
}

TEST(SuitableStruct, PrePostOperations_Binary_Partial)
{
    Struct3 initial, deserialized;
    initial.a = 100;

    // Test save (no hooks)
    auto buffer = SuitableStruct::ssSave(initial);

    // Test load (only after load hook)
    ssLoad(buffer, deserialized);
    ASSERT_TRUE(deserialized.afterLoadCalled);
    ASSERT_EQ(deserialized.a, initial.a);
}

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY

TEST(SuitableStruct, PrePostOperations_Json)
{
    Struct1 initial, deserialized;
    initial.a = 100;

    // Test save hooks
    auto json = SuitableStruct::ssJsonSave(initial);
    ASSERT_TRUE(initial.beforeSaveCalled);
    ASSERT_TRUE(initial.afterSaveCalled);

    // Test load hooks
    SuitableStruct::ssJsonLoad(json, deserialized);
    ASSERT_TRUE(deserialized.beforeLoadCalled);
    ASSERT_TRUE(deserialized.afterLoadCalled);
    ASSERT_EQ(deserialized.a, initial.a);
}

TEST(SuitableStruct, PrePostOperations_Json_Nested)
{
    Struct2 initial, deserialized;
    initial.s1.a = 100;

    // Test save hooks
    auto json = SuitableStruct::ssJsonSave(initial);
    ASSERT_TRUE(initial.beforeSaveCalled);
    ASSERT_TRUE(initial.afterSaveCalled);
    ASSERT_TRUE(initial.s1.beforeSaveCalled);
    ASSERT_TRUE(initial.s1.afterSaveCalled);

    // Test load hooks
    SuitableStruct::ssJsonLoad(json, deserialized);
    ASSERT_TRUE(deserialized.beforeLoadCalled);
    ASSERT_TRUE(deserialized.afterLoadCalled);
    ASSERT_TRUE(deserialized.s1.beforeLoadCalled);
    ASSERT_TRUE(deserialized.s1.afterLoadCalled);
    ASSERT_EQ(deserialized.s1.a, initial.s1.a);
}

TEST(SuitableStruct, PrePostOperations_Json_Partial)
{
    Struct3 initial, deserialized;
    initial.a = 100;

    // Test save (no hooks)
    auto json = SuitableStruct::ssJsonSave(initial);

    // Test load (only after load hook)
    SuitableStruct::ssJsonLoad(json, deserialized);
    ASSERT_TRUE(deserialized.afterLoadCalled);
    ASSERT_EQ(deserialized.a, initial.a);
}

#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
