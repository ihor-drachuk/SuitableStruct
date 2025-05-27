/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

// PRE/POST OPERATION HANDLERS DOCUMENTATION
// ========================================
//
// SuitableStruct provides hooks that are called before and after serialization/deserialization
// operations. These handlers allow you to perform custom logic at specific points in the
// serialization lifecycle.
//
// AVAILABLE HANDLERS:
// ------------------
//
// 1. SS_DEFINE_BEFORE_SAVE_CONST() - Called before serialization begins
//    - Use for: validation, logging, preparing data for serialization
//    - Called on the original object being serialized
//    - Must be const since it doesn't modify the object's serializable state
//
// 2. SS_DEFINE_AFTER_SAVE_CONST() - Called after serialization completes
//    - Use for: cleanup, logging, post-serialization tasks
//    - Called on the original object that was serialized
//    - Must be const since serialization is complete
//
// 3. SS_DEFINE_BEFORE_LOAD() - Called before deserialization begins
//    - Use for: initialization, preparing object for data loading
//    - Called on the target object before any data is loaded into it
//    - Can modify the object since it's being prepared for loading
//
// 4. SS_DEFINE_AFTER_LOAD() - Called after deserialization completes
//    - Use for: validation, post-processing, derived field calculation
//    - Called on the target object after all data has been loaded
//    - Can modify the object for final setup/validation
//
// USAGE EXAMPLE:
// -------------
//
// struct MyStruct {
//     int data{};
//     mutable int saveCount{};  // mutable for const save handlers
//     int loadCount{};
//
//     auto ssTuple() const { return std::tie(data); }
//
//     SS_DEFINE_BEFORE_SAVE_CONST() { saveCount++; }
//     SS_DEFINE_AFTER_SAVE_CONST() { /* cleanup */ }
//     SS_DEFINE_BEFORE_LOAD() { loadCount = 0; }
//     SS_DEFINE_AFTER_LOAD() { loadCount++; }
// };
//
// BEHAVIOR IN DIFFERENT SCENARIOS:
// --------------------------------
//
// 1. SIMPLE SERIALIZATION/DESERIALIZATION:
//    - Save: beforeSave() -> serialize -> afterSave()
//    - Load: beforeLoad() -> deserialize -> afterLoad()
//    - Each handler called exactly once
//
// 2. NESTED STRUCTURES:
//    - Handlers called for each struct in the hierarchy
//    - Outer struct handlers called first, then inner struct handlers
//    - Each struct's handlers called independently
//
// 3. VERSION UPGRADES (e.g., v0 -> v2):
//    - Save: Only source version (v0) handlers called
//      * v0.beforeSave() -> serialize v0 -> v0.afterSave()
//    - Load: Only target version (v2) handlers called
//      * v2.beforeLoad() -> deserialize v0 -> upgrade v0->v1->v2 -> v2.afterLoad()
//
// 4. VERSION DOWNGRADES (e.g., v2 -> v0):
//    - Save: Only source version (v2) handlers called
//      * v2.beforeSave() -> serialize v2, downgrade v2->v1->v0 -> v2.afterSave()
//    - Load: Only target version (v0) handlers called
//      * v0.beforeLoad() -> deserialize v0 -> v0.afterLoad()
//
// CUSTOM HANDLERS LIMITATION:
// ---------------------------
//
// Pre/post operation handlers (SS_DEFINE_BEFORE_SAVE_CONST, SS_DEFINE_AFTER_SAVE_CONST,
// SS_DEFINE_BEFORE_LOAD, SS_DEFINE_AFTER_LOAD) are NOT supported in custom Handlers
// specializations. These handlers must be defined directly in the struct using the
// provided macros. If you need pre/post operations with custom serialization logic,
// define the handlers in the struct and implement custom ssSaveImpl/ssLoadImpl methods
// in the struct itself rather than using Handlers specialization.
//
// IMPORTANT NOTES:
// ---------------
//
// - Save handlers (before/after save) are called during serialization
// - Load handlers (before/after load) are called during deserialization
// - In versioning scenarios, only the final target object gets load handlers called
// - Intermediate version objects created during upgrade/downgrade do NOT trigger load handlers
// - Handlers are optional - define only the ones you need
// - Save handlers must be const, load handlers can be non-const
// - Use mutable members if you need to modify state in const save handlers

#include <gtest/gtest.h>

#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/SerializerJson.h>

namespace {

struct Struct1
{
    int a{};
    mutable int beforeSaveCalled{};
    mutable int afterSaveCalled{};
    int beforeLoadCalled{};
    int afterLoadCalled{};

    auto ssTuple() const { return std::tie(a); }
    auto ssNamesTuple() const { return std::tie("a"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct1)

    SS_DEFINE_BEFORE_SAVE_CONST() { beforeSaveCalled++; }
    SS_DEFINE_AFTER_SAVE_CONST() { afterSaveCalled++; }
    SS_DEFINE_BEFORE_LOAD() { beforeLoadCalled++; }
    SS_DEFINE_AFTER_LOAD() { afterLoadCalled++; }
};

struct Struct2
{
    Struct1 s1{};
    mutable int beforeSaveCalled{};
    mutable int afterSaveCalled{};
    int beforeLoadCalled{};
    int afterLoadCalled{};

    auto ssTuple() const { return std::tie(s1); }
    auto ssNamesTuple() const { return std::tie("s1"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct2)

    SS_DEFINE_BEFORE_SAVE_CONST() { beforeSaveCalled++; }
    SS_DEFINE_AFTER_SAVE_CONST() { afterSaveCalled++; }
    SS_DEFINE_BEFORE_LOAD() { beforeLoadCalled++; }
    SS_DEFINE_AFTER_LOAD() { afterLoadCalled++; }
};

struct Struct3
{
    int a{};
    int afterLoadCalled{};

    auto ssTuple() const { return std::tie(a); }
    auto ssNamesTuple() const { return std::tie("a"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct3)

    SS_DEFINE_AFTER_LOAD() { afterLoadCalled++; }
};

// Versioned structs for testing pre/post operations during versioning
struct VersionedStruct_v0
{
    int value{};
    mutable int beforeSaveCalled{};
    mutable int afterSaveCalled{};
    int beforeLoadCalled{};
    int afterLoadCalled{};

    using ssVersions = std::tuple<VersionedStruct_v0>;
    auto ssTuple() const { return std::tie(value); }
    auto ssNamesTuple() const { return std::tie("value"); }
    SS_COMPARISONS_MEMBER(VersionedStruct_v0)

    SS_DEFINE_BEFORE_SAVE_CONST() { beforeSaveCalled++; }
    SS_DEFINE_AFTER_SAVE_CONST() { afterSaveCalled++; }
    SS_DEFINE_BEFORE_LOAD() { beforeLoadCalled++; }
    SS_DEFINE_AFTER_LOAD() { afterLoadCalled++; }
};

struct VersionedStruct_v1
{
    int value{};
    float extra{};
    mutable int beforeSaveCalled{};
    mutable int afterSaveCalled{};
    int beforeLoadCalled{};
    int afterLoadCalled{};

    using ssVersions = std::tuple<VersionedStruct_v0, VersionedStruct_v1>;
    auto ssTuple() const { return std::tie(value, extra); }
    auto ssNamesTuple() const { return std::tie("value", "extra"); }

    void ssUpgradeFrom(const VersionedStruct_v0& prev) {
        value = prev.value;
        extra = 1.5f;
    }

    void ssDowngradeTo(VersionedStruct_v0& next) const {
        next.value = value;
    }

    SS_COMPARISONS_MEMBER(VersionedStruct_v1)

    SS_DEFINE_BEFORE_SAVE_CONST() { beforeSaveCalled++; }
    SS_DEFINE_AFTER_SAVE_CONST() { afterSaveCalled++; }
    SS_DEFINE_BEFORE_LOAD() { beforeLoadCalled++; }
    SS_DEFINE_AFTER_LOAD() { afterLoadCalled++; }
};

// Global counters for intermediate version testing
namespace {
    int g_v0_beforeSave = 0;
    int g_v0_afterSave = 0;
    int g_v0_beforeLoad = 0;
    int g_v0_afterLoad = 0;

    int g_v1_beforeSave = 0;
    int g_v1_afterSave = 0;
    int g_v1_beforeLoad = 0;
    int g_v1_afterLoad = 0;

    int g_v2_beforeSave = 0;
    int g_v2_afterSave = 0;
    int g_v2_beforeLoad = 0;
    int g_v2_afterLoad = 0;

    void resetCounters() {
        g_v0_beforeSave = g_v0_afterSave = g_v0_beforeLoad = g_v0_afterLoad = 0;
        g_v1_beforeSave = g_v1_afterSave = g_v1_beforeLoad = g_v1_afterLoad = 0;
        g_v2_beforeSave = g_v2_afterSave = g_v2_beforeLoad = g_v2_afterLoad = 0;
    }
}

// Versioned structs with global counters for intermediate version testing
struct IntermediateStruct_v0
{
    int value{};

    using ssVersions = std::tuple<IntermediateStruct_v0>;
    auto ssTuple() const { return std::tie(value); }
    auto ssNamesTuple() const { return std::tie("value"); }
    SS_COMPARISONS_MEMBER(IntermediateStruct_v0)

    SS_DEFINE_BEFORE_SAVE_CONST() { g_v0_beforeSave++; }
    SS_DEFINE_AFTER_SAVE_CONST() { g_v0_afterSave++; }
    SS_DEFINE_BEFORE_LOAD() { g_v0_beforeLoad++; }
    SS_DEFINE_AFTER_LOAD() { g_v0_afterLoad++; }
};

struct IntermediateStruct_v1
{
    int value{};
    float extra{};

    using ssVersions = std::tuple<IntermediateStruct_v0, IntermediateStruct_v1>;
    auto ssTuple() const { return std::tie(value, extra); }
    auto ssNamesTuple() const { return std::tie("value", "extra"); }

    void ssUpgradeFrom(const IntermediateStruct_v0& prev) {
        value = prev.value;
        extra = 1.5f;
    }

    void ssDowngradeTo(IntermediateStruct_v0& next) const {
        next.value = value;
    }

    SS_COMPARISONS_MEMBER(IntermediateStruct_v1)

    SS_DEFINE_BEFORE_SAVE_CONST() { g_v1_beforeSave++; }
    SS_DEFINE_AFTER_SAVE_CONST() { g_v1_afterSave++; }
    SS_DEFINE_BEFORE_LOAD() { g_v1_beforeLoad++; }
    SS_DEFINE_AFTER_LOAD() { g_v1_afterLoad++; }
};

struct IntermediateStruct_v2
{
    int value{};
    double extra{};
    std::string tag;

    using ssVersions = std::tuple<IntermediateStruct_v0, IntermediateStruct_v1, IntermediateStruct_v2>;
    auto ssTuple() const { return std::tie(value, extra, tag); }
    auto ssNamesTuple() const { return std::tie("value", "extra", "tag"); }

    void ssUpgradeFrom(const IntermediateStruct_v1& prev) {
        value = prev.value;
        extra = static_cast<double>(prev.extra);
        tag = "upgraded";
    }

    void ssDowngradeTo(IntermediateStruct_v1& next) const {
        next.value = value;
        next.extra = static_cast<float>(extra);
    }

    SS_COMPARISONS_MEMBER(IntermediateStruct_v2)

    SS_DEFINE_BEFORE_SAVE_CONST() { g_v2_beforeSave++; }
    SS_DEFINE_AFTER_SAVE_CONST() { g_v2_afterSave++; }
    SS_DEFINE_BEFORE_LOAD() { g_v2_beforeLoad++; }
    SS_DEFINE_AFTER_LOAD() { g_v2_afterLoad++; }
};

} // namespace

TEST(SuitableStruct, PrePostOperations_Binary)
{
    Struct1 initial, deserialized;
    initial.a = 100;

    // Test save hooks
    const auto buffer = SuitableStruct::ssSave(initial);
    ASSERT_EQ(initial.beforeSaveCalled, 1);
    ASSERT_EQ(initial.afterSaveCalled, 1);

    // Test load hooks
    ssLoad(buffer, deserialized);
    ASSERT_EQ(deserialized.beforeLoadCalled, 1);
    ASSERT_EQ(deserialized.afterLoadCalled, 1);
    ASSERT_EQ(deserialized.a, initial.a);
}

TEST(SuitableStruct, PrePostOperations_Binary_Nested)
{
    Struct2 initial, deserialized;
    initial.s1.a = 100;

    // Test save hooks
    const auto buffer = SuitableStruct::ssSave(initial);
    ASSERT_EQ(initial.beforeSaveCalled, 1);
    ASSERT_EQ(initial.afterSaveCalled, 1);
    ASSERT_EQ(initial.s1.beforeSaveCalled, 1);
    ASSERT_EQ(initial.s1.afterSaveCalled, 1);

    // Test load hooks
    ssLoad(buffer, deserialized);
    ASSERT_EQ(deserialized.beforeLoadCalled, 1);
    ASSERT_EQ(deserialized.afterLoadCalled, 1);
    ASSERT_EQ(deserialized.s1.beforeLoadCalled, 1);
    ASSERT_EQ(deserialized.s1.afterLoadCalled, 1);
    ASSERT_EQ(deserialized.s1.a, initial.s1.a);
}

TEST(SuitableStruct, PrePostOperations_Binary_Partial)
{
    Struct3 initial, deserialized;
    initial.a = 100;

    // Test save (no hooks)
    const auto buffer = SuitableStruct::ssSave(initial);

    // Test load (only after load hook)
    ssLoad(buffer, deserialized);
    ASSERT_EQ(deserialized.afterLoadCalled, 1);
    ASSERT_EQ(deserialized.a, initial.a);
}

TEST(SuitableStruct, PrePostOperations_VersionUpgrade)
{
    VersionedStruct_v0 initial;
    initial.value = 100;

    // Test save hooks for v0
    const auto buffer = SuitableStruct::ssSave(initial);
    ASSERT_EQ(initial.beforeSaveCalled, 1);
    ASSERT_EQ(initial.afterSaveCalled, 1);

    // Test load hooks during upgrade to v1
    VersionedStruct_v1 upgraded;
    ssLoad(buffer, upgraded);
    ASSERT_EQ(upgraded.beforeLoadCalled, 1);
    ASSERT_EQ(upgraded.afterLoadCalled, 1);
    ASSERT_EQ(upgraded.value, initial.value);
    ASSERT_FLOAT_EQ(upgraded.extra, 1.5f);
}

TEST(SuitableStruct, PrePostOperations_VersionDowngrade)
{
    VersionedStruct_v1 initial;
    initial.value = 100;
    initial.extra = 3.14f;

    // Test save hooks for v1
    const auto buffer = SuitableStruct::ssSave(initial);
    ASSERT_EQ(initial.beforeSaveCalled, 1);
    ASSERT_EQ(initial.afterSaveCalled, 1);

    // Test load hooks during downgrade to v0
    VersionedStruct_v0 downgraded;
    ssLoad(buffer, downgraded);
    ASSERT_EQ(downgraded.beforeLoadCalled, 1);
    ASSERT_EQ(downgraded.afterLoadCalled, 1);
    ASSERT_EQ(downgraded.value, initial.value);
}

TEST(SuitableStruct, PrePostOperations_IntermediateVersionUpgrade)
{
    resetCounters();
    IntermediateStruct_v0 initial;
    initial.value = 100;

    // Test save hooks for v0
    const auto buffer = SuitableStruct::ssSave(initial);
    ASSERT_EQ(g_v0_beforeSave, 1);
    ASSERT_EQ(g_v0_afterSave, 1);

    // Test load hooks during upgrade to v2 (should go through v1)
    IntermediateStruct_v2 upgraded;
    ssLoad(buffer, upgraded);

    // Only the final target version (v2) should have pre/post load hooks called
    ASSERT_EQ(g_v2_beforeLoad, 1);
    ASSERT_EQ(g_v2_afterLoad, 1);

    // Intermediate versions (v0, v1) should NOT have load hooks called
    // because they are created via ssUpgradeFrom, not through the load process
    ASSERT_EQ(g_v0_beforeLoad, 0);
    ASSERT_EQ(g_v0_afterLoad, 0);
    ASSERT_EQ(g_v1_beforeLoad, 0);
    ASSERT_EQ(g_v1_afterLoad, 0);

    // Verify data was correctly transferred through versions
    ASSERT_EQ(upgraded.value, initial.value);
    ASSERT_FLOAT_EQ(upgraded.extra, 1.5f);  // From v1 upgrade
    ASSERT_EQ(upgraded.tag, "upgraded");    // From v2 upgrade
}

TEST(SuitableStruct, PrePostOperations_IntermediateVersionDowngrade)
{
    resetCounters();
    IntermediateStruct_v2 initial;
    initial.value = 100;
    initial.extra = 3.14;
    initial.tag = "test";

    // Test save hooks for v2
    const auto buffer = SuitableStruct::ssSave(initial);
    ASSERT_EQ(g_v2_beforeSave, 1);
    ASSERT_EQ(g_v2_afterSave, 1);

    // Test load hooks during downgrade to v0 (should go through v1)
    IntermediateStruct_v0 downgraded;
    ssLoad(buffer, downgraded);

    // Only the final target version (v0) should have pre/post load hooks called
    ASSERT_EQ(g_v0_beforeLoad, 1);
    ASSERT_EQ(g_v0_afterLoad, 1);

    // Intermediate versions (v1, v2) should NOT have load hooks called
    // because they are created via ssDowngradeTo, not through the load process
    ASSERT_EQ(g_v1_beforeLoad, 0);
    ASSERT_EQ(g_v1_afterLoad, 0);
    ASSERT_EQ(g_v2_beforeLoad, 0);
    ASSERT_EQ(g_v2_afterLoad, 0);

    // Verify data was correctly transferred through versions
    ASSERT_EQ(downgraded.value, initial.value);
}

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY

TEST(SuitableStruct, PrePostOperations_Json)
{
    Struct1 initial, deserialized;
    initial.a = 100;

    // Test save hooks
    const auto json = SuitableStruct::ssJsonSave(initial);
    ASSERT_EQ(initial.beforeSaveCalled, 1);
    ASSERT_EQ(initial.afterSaveCalled, 1);

    // Test load hooks
    SuitableStruct::ssJsonLoad(json, deserialized);
    ASSERT_EQ(deserialized.beforeLoadCalled, 1);
    ASSERT_EQ(deserialized.afterLoadCalled, 1);
    ASSERT_EQ(deserialized.a, initial.a);
}

TEST(SuitableStruct, PrePostOperations_Json_Nested)
{
    Struct2 initial, deserialized;
    initial.s1.a = 100;

    // Test save hooks
    const auto json = SuitableStruct::ssJsonSave(initial);
    ASSERT_EQ(initial.beforeSaveCalled, 1);
    ASSERT_EQ(initial.afterSaveCalled, 1);
    ASSERT_EQ(initial.s1.beforeSaveCalled, 1);
    ASSERT_EQ(initial.s1.afterSaveCalled, 1);

    // Test load hooks
    SuitableStruct::ssJsonLoad(json, deserialized);
    ASSERT_EQ(deserialized.beforeLoadCalled, 1);
    ASSERT_EQ(deserialized.afterLoadCalled, 1);
    ASSERT_EQ(deserialized.s1.beforeLoadCalled, 1);
    ASSERT_EQ(deserialized.s1.afterLoadCalled, 1);
    ASSERT_EQ(deserialized.s1.a, initial.s1.a);
}

TEST(SuitableStruct, PrePostOperations_Json_Partial)
{
    Struct3 initial, deserialized;
    initial.a = 100;

    // Test save (no hooks)
    const auto json = SuitableStruct::ssJsonSave(initial);

    // Test load (only after load hook)
    SuitableStruct::ssJsonLoad(json, deserialized);
    ASSERT_EQ(deserialized.afterLoadCalled, 1);
    ASSERT_EQ(deserialized.a, initial.a);
}

TEST(SuitableStruct, PrePostOperations_JsonIntermediateVersionUpgrade)
{
    resetCounters();
    IntermediateStruct_v0 initial;
    initial.value = 100;

    // Test save hooks for v0
    const auto json = SuitableStruct::ssJsonSave(initial);
    ASSERT_EQ(g_v0_beforeSave, 1);
    ASSERT_EQ(g_v0_afterSave, 1);

    // Test load hooks during upgrade to v2 (should go through v1)
    IntermediateStruct_v2 upgraded;
    SuitableStruct::ssJsonLoad(json, upgraded);

    // Only the final target version (v2) should have pre/post load hooks called
    ASSERT_EQ(g_v2_beforeLoad, 1);
    ASSERT_EQ(g_v2_afterLoad, 1);

    // Intermediate versions (v0, v1) should NOT have load hooks called
    // because they are created via ssUpgradeFrom, not through the load process
    ASSERT_EQ(g_v0_beforeLoad, 0);
    ASSERT_EQ(g_v0_afterLoad, 0);
    ASSERT_EQ(g_v1_beforeLoad, 0);
    ASSERT_EQ(g_v1_afterLoad, 0);

    // Verify data was correctly transferred through versions
    ASSERT_EQ(upgraded.value, initial.value);
    ASSERT_FLOAT_EQ(upgraded.extra, 1.5f);  // From v1 upgrade
    ASSERT_EQ(upgraded.tag, "upgraded");    // From v2 upgrade
}

TEST(SuitableStruct, PrePostOperations_JsonIntermediateVersionDowngrade)
{
    resetCounters();
    IntermediateStruct_v2 initial;
    initial.value = 100;
    initial.extra = 3.14;
    initial.tag = "test";

    // Test save hooks for v2
    const auto json = SuitableStruct::ssJsonSave(initial);
    ASSERT_EQ(g_v2_beforeSave, 1);
    ASSERT_EQ(g_v2_afterSave, 1);

    // Test load hooks during downgrade to v0 (should go through v1)
    IntermediateStruct_v0 downgraded;
    SuitableStruct::ssJsonLoad(json, downgraded);

    // Only the final target version (v0) should have pre/post load hooks called
    ASSERT_EQ(g_v0_beforeLoad, 1);
    ASSERT_EQ(g_v0_afterLoad, 1);

    // Intermediate versions (v1, v2) should NOT have load hooks called
    // because they are created via ssDowngradeTo, not through the load process
    ASSERT_EQ(g_v1_beforeLoad, 0);
    ASSERT_EQ(g_v1_afterLoad, 0);
    ASSERT_EQ(g_v2_beforeLoad, 0);
    ASSERT_EQ(g_v2_afterLoad, 0);

    // Verify data was correctly transferred through versions
    ASSERT_EQ(downgraded.value, initial.value);
}

// Test to ensure binary and JSON serialization have identical hook call behavior
TEST(SuitableStruct, PrePostOperations_BinaryJsonConsistency)
{
    resetCounters();

    IntermediateStruct_v1 binaryObj;
    binaryObj.value = 42;
    binaryObj.extra = 3.14f;

    IntermediateStruct_v1 jsonObj;
    jsonObj.value = 42;
    jsonObj.extra = 3.14f;

    // Serialize with both formats
    SuitableStruct::ssSave(binaryObj);
    SuitableStruct::ssJsonSave(jsonObj);

    // Both should have called hooks the same number of times
    // For v1 object with 2 versions (v0, v1), hooks should be called twice:
    // - Once for v1 serialization (g_v1_beforeSave, g_v1_afterSave)
    // - Once for v0 downgrade serialization (g_v0_beforeSave, g_v0_afterSave)
    ASSERT_EQ(g_v1_beforeSave, 2);
    ASSERT_EQ(g_v1_afterSave, 2);
    ASSERT_EQ(g_v0_beforeSave, 2);
    ASSERT_EQ(g_v0_afterSave, 2);
}

#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
