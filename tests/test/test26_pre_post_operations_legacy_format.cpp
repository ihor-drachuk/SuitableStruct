/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

// Tests that before/after save/load hooks are called the correct number of times
// when loading data serialized in the legacy F0 format.
//
// Covers the bug where ssLoadInternalRet and ssLoadImplViaTupleInternal
// in the F0 legacy path did not call ssBeforeLoadImpl/ssAfterLoadImpl for
// nested class objects, while the F1 path did.
//
// Scenarios covered:
//   1. Simple struct (direct save/load)
//   2. Nested struct (class field inside class — ssLoadImplViaTupleInternal path)
//   3. Three-level nesting (deep recursion test)
//   4. Optional<class> (ssLoadInternalRet path)
//   5. Variant<monostate, class> (ssLoadInternalRet path)
//   6. F1 vs F0 symmetry: exact same hook counts for identical data

#include <gtest/gtest.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Internals/Helpers.h>
#include <SuitableStruct/Hashes.h>

#include <optional>
#include <variant>

using namespace SuitableStruct;

// ============================================================
// Test structs
// ============================================================

namespace {

// --- Level 0: leaf struct ---
struct Leaf
{
    int val{};
    mutable int beforeSave{};
    mutable int afterSave{};
    int beforeLoad{};
    int afterLoad{};

    auto ssTuple() const { return std::tie(val); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Leaf)

    SS_DEFINE_BEFORE_SAVE_CONST() { beforeSave++; }
    SS_DEFINE_AFTER_SAVE_CONST() { afterSave++; }
    SS_DEFINE_BEFORE_LOAD() { beforeLoad++; }
    SS_DEFINE_AFTER_LOAD() { afterLoad++; }
};

// --- Level 1: contains Leaf ---
struct Mid
{
    Leaf leaf{};
    int extra{};
    mutable int beforeSave{};
    mutable int afterSave{};
    int beforeLoad{};
    int afterLoad{};

    auto ssTuple() const { return std::tie(leaf, extra); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Mid)

    SS_DEFINE_BEFORE_SAVE_CONST() { beforeSave++; }
    SS_DEFINE_AFTER_SAVE_CONST() { afterSave++; }
    SS_DEFINE_BEFORE_LOAD() { beforeLoad++; }
    SS_DEFINE_AFTER_LOAD() { afterLoad++; }
};

// --- Level 2: contains Mid ---
struct Root
{
    Mid mid{};
    int top{};
    mutable int beforeSave{};
    mutable int afterSave{};
    int beforeLoad{};
    int afterLoad{};

    auto ssTuple() const { return std::tie(mid, top); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Root)

    SS_DEFINE_BEFORE_SAVE_CONST() { beforeSave++; }
    SS_DEFINE_AFTER_SAVE_CONST() { afterSave++; }
    SS_DEFINE_BEFORE_LOAD() { beforeLoad++; }
    SS_DEFINE_AFTER_LOAD() { afterLoad++; }
};

// --- Struct with optional<class> field ---
struct WithOptional
{
    std::optional<Leaf> opt;
    mutable int beforeSave{};
    mutable int afterSave{};
    int beforeLoad{};
    int afterLoad{};

    auto ssTuple() const { return std::tie(opt); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(WithOptional)

    SS_DEFINE_BEFORE_SAVE_CONST() { beforeSave++; }
    SS_DEFINE_AFTER_SAVE_CONST() { afterSave++; }
    SS_DEFINE_BEFORE_LOAD() { beforeLoad++; }
    SS_DEFINE_AFTER_LOAD() { afterLoad++; }
};

// --- Struct with variant<monostate, class> field ---
struct WithVariant
{
    std::variant<std::monostate, Leaf> var;
    mutable int beforeSave{};
    mutable int afterSave{};
    int beforeLoad{};
    int afterLoad{};

    auto ssTuple() const { return std::tie(var); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(WithVariant)

    SS_DEFINE_BEFORE_SAVE_CONST() { beforeSave++; }
    SS_DEFINE_AFTER_SAVE_CONST() { afterSave++; }
    SS_DEFINE_BEFORE_LOAD() { beforeLoad++; }
    SS_DEFINE_AFTER_LOAD() { afterLoad++; }
};


// ============================================================
// F0 buffer construction helpers
// ============================================================

// Serialize raw ssTuple fields the way old F0 ssSaveImpl did (no segments).
// For each class-type tuple element: write version(0) + recurse.
// For primitive tuple elements: write raw, no version byte.

Buffer serializeF0_Leaf(const Leaf& obj) {
    Buffer buf;
    buf += ssSaveImpl(obj.val);
    return buf;
}

Buffer serializeF0_Mid(const Mid& obj) {
    Buffer buf;
    // leaf is class: version byte + fields
    buf.write(static_cast<uint8_t>(0));
    buf += serializeF0_Leaf(obj.leaf);
    // extra is primitive: no version byte
    buf += ssSaveImpl(obj.extra);
    return buf;
}

Buffer serializeF0_Root(const Root& obj) {
    Buffer buf;
    // mid is class: version byte + fields
    buf.write(static_cast<uint8_t>(0));
    buf += serializeF0_Mid(obj.mid);
    // top is primitive
    buf += ssSaveImpl(obj.top);
    return buf;
}

Buffer serializeF0_WithOptional(const WithOptional& obj) {
    Buffer buf;
    // In F0 format, std::optional is a class type, so ssLoadImplViaTupleInternal
    // reads a version byte for it first, then calls ssLoadAndConvert which calls
    // ssLoadImpl(optional<Leaf>). ssLoadImpl for optional reads: bool hasValue,
    // then for the Leaf inside calls ssLoadInternal which (in F0 mode) reads
    // version byte + fields.
    //
    // So the wire format is:
    //   version(0) for optional itself
    //   bool hasValue
    //   [if hasValue] version(0) for Leaf + Leaf fields
    buf.write(static_cast<uint8_t>(0)); // version for std::optional<Leaf> (class type)
    buf += ssSaveImpl(obj.opt.has_value());
    if (obj.opt) {
        buf.write(static_cast<uint8_t>(0)); // version for Leaf
        buf += serializeF0_Leaf(*obj.opt);
    }
    return buf;
}

Buffer serializeF0_WithVariant(const WithVariant& obj) {
    Buffer buf;
    // Same as optional: variant is a class type, gets version byte first.
    // Then ssLoadImpl(variant) reads: uint8_t index + content.
    // For Leaf content (class type), ssLoadInternalRet reads version byte + fields.
    //
    // Wire format:
    //   version(0) for variant itself
    //   uint8_t index
    //   [if Leaf] version(0) for Leaf + Leaf fields
    buf.write(static_cast<uint8_t>(0)); // version for std::variant<...> (class type)
    buf.write(static_cast<uint8_t>(obj.var.index()));
    std::visit([&buf](const auto& x) {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, Leaf>) {
            buf.write(static_cast<uint8_t>(0)); // version for Leaf
            buf += serializeF0_Leaf(x);
        }
        // monostate: nothing to write
    }, obj.var);
    return buf;
}

template<typename SerializeFunc>
Buffer createF0Buffer(SerializeFunc serializeFunc, uint8_t version)
{
    Buffer payload;
    payload.writeRaw(static_cast<const void*>(Internal::SS_FORMAT_F0), sizeof(Internal::SS_FORMAT_F0));
    payload.write(version);
    payload += serializeFunc();

    Buffer header;
    header.write(static_cast<uint64_t>(payload.size()));
    header.write(ssHashRaw_F0(payload.data(), payload.size()));

    return header + payload;
}

} // namespace


// ============================================================
// 1. F1 baseline (verify test infrastructure works)
// ============================================================

TEST(SuitableStruct, PrePostOps_F1_Simple)
{
    Leaf original;
    original.val = 42;

    const auto buf = ssSave(original);
    EXPECT_EQ(original.beforeSave, 1);
    EXPECT_EQ(original.afterSave, 1);

    Leaf loaded;
    ssLoad(buf, loaded);
    EXPECT_EQ(loaded.beforeLoad, 1);
    EXPECT_EQ(loaded.afterLoad, 1);
    EXPECT_EQ(loaded.val, 42);
}

TEST(SuitableStruct, PrePostOps_F1_Nested)
{
    Mid original;
    original.leaf.val = 10;
    original.extra = 20;

    const auto buf = ssSave(original);
    EXPECT_EQ(original.beforeSave, 1);
    EXPECT_EQ(original.afterSave, 1);
    EXPECT_EQ(original.leaf.beforeSave, 1);
    EXPECT_EQ(original.leaf.afterSave, 1);

    Mid loaded;
    ssLoad(buf, loaded);
    EXPECT_EQ(loaded.beforeLoad, 1);
    EXPECT_EQ(loaded.afterLoad, 1);
    EXPECT_EQ(loaded.leaf.beforeLoad, 1);
    EXPECT_EQ(loaded.leaf.afterLoad, 1);
    EXPECT_EQ(loaded.leaf.val, 10);
    EXPECT_EQ(loaded.extra, 20);
}

TEST(SuitableStruct, PrePostOps_F1_DeepNested)
{
    Root original;
    original.mid.leaf.val = 1;
    original.mid.extra = 2;
    original.top = 3;

    const auto buf = ssSave(original);
    EXPECT_EQ(original.beforeSave, 1);
    EXPECT_EQ(original.afterSave, 1);
    EXPECT_EQ(original.mid.beforeSave, 1);
    EXPECT_EQ(original.mid.afterSave, 1);
    EXPECT_EQ(original.mid.leaf.beforeSave, 1);
    EXPECT_EQ(original.mid.leaf.afterSave, 1);

    Root loaded;
    ssLoad(buf, loaded);
    EXPECT_EQ(loaded.beforeLoad, 1);
    EXPECT_EQ(loaded.afterLoad, 1);
    EXPECT_EQ(loaded.mid.beforeLoad, 1);
    EXPECT_EQ(loaded.mid.afterLoad, 1);
    EXPECT_EQ(loaded.mid.leaf.beforeLoad, 1);
    EXPECT_EQ(loaded.mid.leaf.afterLoad, 1);
    EXPECT_EQ(loaded.mid.leaf.val, 1);
    EXPECT_EQ(loaded.mid.extra, 2);
    EXPECT_EQ(loaded.top, 3);
}

TEST(SuitableStruct, PrePostOps_F1_Optional)
{
    WithOptional original;
    original.opt = Leaf{99};

    const auto buf = ssSave(original);
    EXPECT_EQ(original.beforeSave, 1);
    EXPECT_EQ(original.afterSave, 1);
    EXPECT_EQ(original.opt->beforeSave, 1);
    EXPECT_EQ(original.opt->afterSave, 1);

    WithOptional loaded;
    ssLoad(buf, loaded);
    EXPECT_EQ(loaded.beforeLoad, 1);
    EXPECT_EQ(loaded.afterLoad, 1);
    ASSERT_TRUE(loaded.opt.has_value());
    EXPECT_EQ(loaded.opt->beforeLoad, 1);
    EXPECT_EQ(loaded.opt->afterLoad, 1);
    EXPECT_EQ(loaded.opt->val, 99);
}

TEST(SuitableStruct, PrePostOps_F1_Variant)
{
    WithVariant original;
    original.var = Leaf{77};

    const auto buf = ssSave(original);
    EXPECT_EQ(original.beforeSave, 1);
    EXPECT_EQ(original.afterSave, 1);
    EXPECT_EQ(std::get<Leaf>(original.var).beforeSave, 1);
    EXPECT_EQ(std::get<Leaf>(original.var).afterSave, 1);

    WithVariant loaded;
    ssLoad(buf, loaded);
    EXPECT_EQ(loaded.beforeLoad, 1);
    EXPECT_EQ(loaded.afterLoad, 1);
    ASSERT_EQ(loaded.var.index(), 1u);
    EXPECT_EQ(std::get<Leaf>(loaded.var).beforeLoad, 1);
    EXPECT_EQ(std::get<Leaf>(loaded.var).afterLoad, 1);
    EXPECT_EQ(std::get<Leaf>(loaded.var).val, 77);
}


// ============================================================
// 2. F0 legacy format tests
// ============================================================

TEST(SuitableStruct, PrePostOps_F0_Simple)
{
    Leaf original;
    original.val = 42;

    auto buf = createF0Buffer([&]{ return serializeF0_Leaf(original); }, 0);

    Leaf loaded;
    ssLoad(buf, loaded);

    EXPECT_EQ(loaded.beforeLoad, 1);
    EXPECT_EQ(loaded.afterLoad, 1);
    EXPECT_EQ(loaded.val, 42);
}

TEST(SuitableStruct, PrePostOps_F0_Nested)
{
    Mid original;
    original.leaf.val = 10;
    original.extra = 20;

    auto buf = createF0Buffer([&]{ return serializeF0_Mid(original); }, 0);

    Mid loaded;
    ssLoad(buf, loaded);

    // Outer
    EXPECT_EQ(loaded.beforeLoad, 1);
    EXPECT_EQ(loaded.afterLoad, 1);
    // Inner — was broken before fix (both were 0)
    EXPECT_EQ(loaded.leaf.beforeLoad, 1);
    EXPECT_EQ(loaded.leaf.afterLoad, 1);
    EXPECT_EQ(loaded.leaf.val, 10);
    EXPECT_EQ(loaded.extra, 20);
}

TEST(SuitableStruct, PrePostOps_F0_DeepNested)
{
    Root original;
    original.mid.leaf.val = 1;
    original.mid.extra = 2;
    original.top = 3;

    auto buf = createF0Buffer([&]{ return serializeF0_Root(original); }, 0);

    Root loaded;
    ssLoad(buf, loaded);

    // Level 0 (Root)
    EXPECT_EQ(loaded.beforeLoad, 1);
    EXPECT_EQ(loaded.afterLoad, 1);
    // Level 1 (Mid)
    EXPECT_EQ(loaded.mid.beforeLoad, 1);
    EXPECT_EQ(loaded.mid.afterLoad, 1);
    // Level 2 (Leaf) — deepest nesting
    EXPECT_EQ(loaded.mid.leaf.beforeLoad, 1);
    EXPECT_EQ(loaded.mid.leaf.afterLoad, 1);
    EXPECT_EQ(loaded.mid.leaf.val, 1);
    EXPECT_EQ(loaded.mid.extra, 2);
    EXPECT_EQ(loaded.top, 3);
}

TEST(SuitableStruct, PrePostOps_F0_Optional)
{
    WithOptional original;
    original.opt = Leaf{99};

    auto buf = createF0Buffer([&]{ return serializeF0_WithOptional(original); }, 0);

    WithOptional loaded;
    ssLoad(buf, loaded);

    EXPECT_EQ(loaded.beforeLoad, 1);
    EXPECT_EQ(loaded.afterLoad, 1);
    ASSERT_TRUE(loaded.opt.has_value());
    // Leaf inside optional — ssLoadInternalRet path
    EXPECT_EQ(loaded.opt->beforeLoad, 1);
    EXPECT_EQ(loaded.opt->afterLoad, 1);
    EXPECT_EQ(loaded.opt->val, 99);
}

TEST(SuitableStruct, PrePostOps_F0_Variant)
{
    WithVariant original;
    original.var = Leaf{77};

    auto buf = createF0Buffer([&]{ return serializeF0_WithVariant(original); }, 0);

    WithVariant loaded;
    ssLoad(buf, loaded);

    EXPECT_EQ(loaded.beforeLoad, 1);
    EXPECT_EQ(loaded.afterLoad, 1);
    ASSERT_EQ(loaded.var.index(), 1u);
    // Leaf inside variant — ssLoadInternalRet path
    EXPECT_EQ(std::get<Leaf>(loaded.var).beforeLoad, 1);
    EXPECT_EQ(std::get<Leaf>(loaded.var).afterLoad, 1);
    EXPECT_EQ(std::get<Leaf>(loaded.var).val, 77);
}


// ============================================================
// 3. F0 vs F1 symmetry: hook counts must be identical
// ============================================================

TEST(SuitableStruct, PrePostOps_F0F1_Symmetry_Nested)
{
    Mid original;
    original.leaf.val = 10;
    original.extra = 20;

    // F1
    Mid f1Loaded;
    ssLoad(ssSave(original), f1Loaded);

    // F0
    Mid f0Loaded;
    auto f0Buf = createF0Buffer([&]{ return serializeF0_Mid(original); }, 0);
    ssLoad(f0Buf, f0Loaded);

    // Hook counts must match exactly between F0 and F1
    EXPECT_EQ(f0Loaded.beforeLoad, f1Loaded.beforeLoad);
    EXPECT_EQ(f0Loaded.afterLoad, f1Loaded.afterLoad);
    EXPECT_EQ(f0Loaded.leaf.beforeLoad, f1Loaded.leaf.beforeLoad);
    EXPECT_EQ(f0Loaded.leaf.afterLoad, f1Loaded.leaf.afterLoad);
}

TEST(SuitableStruct, PrePostOps_F0F1_Symmetry_DeepNested)
{
    Root original;
    original.mid.leaf.val = 1;
    original.mid.extra = 2;
    original.top = 3;

    // F1
    Root f1Loaded;
    ssLoad(ssSave(original), f1Loaded);

    // F0
    Root f0Loaded;
    auto f0Buf = createF0Buffer([&]{ return serializeF0_Root(original); }, 0);
    ssLoad(f0Buf, f0Loaded);

    // All 3 levels must have identical hook counts
    EXPECT_EQ(f0Loaded.beforeLoad, f1Loaded.beforeLoad);
    EXPECT_EQ(f0Loaded.afterLoad, f1Loaded.afterLoad);
    EXPECT_EQ(f0Loaded.mid.beforeLoad, f1Loaded.mid.beforeLoad);
    EXPECT_EQ(f0Loaded.mid.afterLoad, f1Loaded.mid.afterLoad);
    EXPECT_EQ(f0Loaded.mid.leaf.beforeLoad, f1Loaded.mid.leaf.beforeLoad);
    EXPECT_EQ(f0Loaded.mid.leaf.afterLoad, f1Loaded.mid.leaf.afterLoad);
}
