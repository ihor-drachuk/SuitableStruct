/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

// Tests for ssVersionOffset — allows dropping old version structs from codebase.
// Wire format uses absolute version indices (offset + tuple position).

#include <gtest/gtest.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Exceptions.h>

using namespace SuitableStruct;

namespace {

// ============================================================
// Full history structs (offset=0, the default)
// ============================================================

struct Full_v0 {
    int a{};
    using ssVersions = std::tuple<Full_v0>;
    auto ssTuple() const { return std::tie(a); }
    auto ssNamesTuple() const { return std::tie("a"); }
    SS_COMPARISONS_MEMBER(Full_v0)
};

struct Full_v1 {
    int a{};
    float b{};
    using ssVersions = std::tuple<Full_v0, Full_v1>;
    auto ssTuple() const { return std::tie(a, b); }
    auto ssNamesTuple() const { return std::tie("a", "b"); }
    SS_COMPARISONS_MEMBER(Full_v1)

    void ssUpgradeFrom(const Full_v0& prev) { a = prev.a; b = 1.5f; }
    void ssDowngradeTo(Full_v0& next) const { next.a = a; }
};

struct Full_v2 {
    int a{};
    float b{};
    std::string c;
    using ssVersions = std::tuple<Full_v0, Full_v1, Full_v2>;
    auto ssTuple() const { return std::tie(a, b, c); }
    auto ssNamesTuple() const { return std::tie("a", "b", "c"); }
    SS_COMPARISONS_MEMBER(Full_v2)

    void ssUpgradeFrom(const Full_v1& prev) { a = prev.a; b = prev.b; c = "default"; }
    void ssDowngradeTo(Full_v1& next) const { next.a = a; next.b = b; }
};

struct Full_v3 {
    int a{};
    float b{};
    std::string c;
    int d{};
    using ssVersions = std::tuple<Full_v0, Full_v1, Full_v2, Full_v3>;
    auto ssTuple() const { return std::tie(a, b, c, d); }
    auto ssNamesTuple() const { return std::tie("a", "b", "c", "d"); }
    SS_COMPARISONS_MEMBER(Full_v3)

    void ssUpgradeFrom(const Full_v2& prev) { a = prev.a; b = prev.b; c = prev.c; d = 42; }
    void ssDowngradeTo(Full_v2& next) const { next.a = a; next.b = b; next.c = c; }
};

// ============================================================
// Truncated history structs (offset=2, V0 and V1 dropped)
// Uses the SAME serialized field layout as Full_v2/v3
// ============================================================

// This is equivalent to Full_v2, but the reader doesn't know about v0/v1
struct Trunc_v2 {
    int a{};
    float b{};
    std::string c;
    using ssVersions = std::tuple<Trunc_v2>; // Only knows itself
    static constexpr uint8_t ssVersionOffset = 2;
    auto ssTuple() const { return std::tie(a, b, c); }
    auto ssNamesTuple() const { return std::tie("a", "b", "c"); }
    SS_COMPARISONS_MEMBER(Trunc_v2)
};

// This is equivalent to Full_v3, but only knows v2 and v3
struct Trunc_v3 {
    int a{};
    float b{};
    std::string c;
    int d{};
    using ssVersions = std::tuple<Trunc_v2, Trunc_v3>;
    static constexpr uint8_t ssVersionOffset = 2;
    auto ssTuple() const { return std::tie(a, b, c, d); }
    auto ssNamesTuple() const { return std::tie("a", "b", "c", "d"); }
    SS_COMPARISONS_MEMBER(Trunc_v3)

    void ssUpgradeFrom(const Trunc_v2& prev) { a = prev.a; b = prev.b; c = prev.c; d = 42; }
    void ssDowngradeTo(Trunc_v2& next) const { next.a = a; next.b = b; next.c = c; }
};

// ============================================================
// Helpers
// ============================================================

uint8_t countBinarySegments(const Buffer& buffer)
{
    BufferReader reader(buffer);
    reader.advance(sizeof(uint64_t)); // size
    reader.advance(sizeof(uint32_t)); // hash
    reader.advance(5);                // format marker
    uint8_t count{};
    reader.read(count);
    return count;
}

// Read first segment's wire version index from binary buffer
uint8_t firstSegmentWireVersion(const Buffer& buffer)
{
    BufferReader reader(buffer);
    reader.advance(sizeof(uint64_t)); // size
    reader.advance(sizeof(uint32_t)); // hash
    reader.advance(5);                // format marker
    reader.advance(1);                // segment count
    uint8_t wireVer{};
    reader.read(wireVer);
    return wireVer;
}

// Read all segment wire versions from binary buffer
std::vector<uint8_t> allSegmentWireVersions(const Buffer& buffer)
{
    BufferReader reader(buffer);
    reader.advance(sizeof(uint64_t)); // size
    reader.advance(sizeof(uint32_t)); // hash
    reader.advance(5);                // format marker
    uint8_t count{};
    reader.read(count);

    std::vector<uint8_t> versions;
    for (uint8_t i = 0; i < count; ++i) {
        uint8_t wireVer{};
        reader.read(wireVer);
        const auto segSize = reader.read<uint64_t>();
        reader.advance(segSize);
        versions.push_back(wireVer);
    }
    return versions;
}

} // namespace


// ============================================================
// Verify SSVersion values are correct
// ============================================================

TEST(SuitableStruct, Offset_SSVersionValues)
{
    // Full history (offset=0)
    EXPECT_EQ(SSVersion<Full_v0>::value, 0u);
    EXPECT_EQ(SSVersion<Full_v1>::value, 1u);
    EXPECT_EQ(SSVersion<Full_v2>::value, 2u);
    EXPECT_EQ(SSVersion<Full_v3>::value, 3u);

    // Truncated history (offset=2)
    EXPECT_EQ(SSVersion<Trunc_v2>::value, 2u); // offset(2) + tuplePos(0)
    EXPECT_EQ(SSVersion<Trunc_v3>::value, 3u); // offset(2) + tuplePos(1)

    // Offsets
    EXPECT_EQ(SSVersionOffset<Full_v0>::value, 0u);
    EXPECT_EQ(SSVersionOffset<Full_v3>::value, 0u);
    EXPECT_EQ(SSVersionOffset<Trunc_v2>::value, 2u);
    EXPECT_EQ(SSVersionOffset<Trunc_v3>::value, 2u);
}

// ============================================================
// Zero offset (default) behaves identically to before
// ============================================================

TEST(SuitableStruct, Offset_Zero)
{
    Full_v2 original;
    original.a = 10;
    original.b = 2.5f;
    original.c = "hello";

    const auto buf = ssSave(original);

    Full_v2 loaded;
    ssLoad(buf, loaded);
    EXPECT_EQ(loaded, original);

    // Wire versions should be 2, 1, 0
    auto versions = allSegmentWireVersions(buf);
    ASSERT_EQ(versions.size(), 3u);
    EXPECT_EQ(versions[0], 2);
    EXPECT_EQ(versions[1], 1);
    EXPECT_EQ(versions[2], 0);
}

// ============================================================
// Basic offset: round-trip with truncated history
// ============================================================

TEST(SuitableStruct, Offset_Basic)
{
    Trunc_v3 original;
    original.a = 42;
    original.b = 3.14f;
    original.c = "test";
    original.d = 99;

    const auto buf = ssSave(original);

    // Wire versions should be 3, 2 (NOT 1, 0)
    auto versions = allSegmentWireVersions(buf);
    ASSERT_EQ(versions.size(), 2u);
    EXPECT_EQ(versions[0], 3);
    EXPECT_EQ(versions[1], 2);

    // Round-trip
    Trunc_v3 loaded;
    ssLoad(buf, loaded);
    EXPECT_EQ(loaded, original);
}

// ============================================================
// Upgrade: save as v2, load as v3 (both with offset)
// ============================================================

TEST(SuitableStruct, Offset_Upgrade)
{
    Trunc_v2 original;
    original.a = 10;
    original.b = 2.5f;
    original.c = "old";

    const auto buf = ssSave(original);

    // Wire version should be 2
    auto versions = allSegmentWireVersions(buf);
    ASSERT_EQ(versions.size(), 1u);
    EXPECT_EQ(versions[0], 2);

    // Load as v3 → upgrade
    Trunc_v3 loaded;
    ssLoad(buf, loaded);
    EXPECT_EQ(loaded.a, 10);
    EXPECT_FLOAT_EQ(loaded.b, 2.5f);
    EXPECT_EQ(loaded.c, "old");
    EXPECT_EQ(loaded.d, 42); // default from ssUpgradeFrom
}

// ============================================================
// Downgrade: save as v3, load as v2 (both with offset)
// ============================================================

TEST(SuitableStruct, Offset_Downgrade)
{
    Trunc_v3 original;
    original.a = 7;
    original.b = 1.0f;
    original.c = "hi";
    original.d = 55;

    const auto buf = ssSave(original);

    Trunc_v2 loaded;
    ssLoad(buf, loaded);
    EXPECT_EQ(loaded.a, 7);
    EXPECT_FLOAT_EQ(loaded.b, 1.0f);
    EXPECT_EQ(loaded.c, "hi");
}

// ============================================================
// Forgotten version: data with v0 → reader with offset=2
// ============================================================

TEST(SuitableStruct, Offset_ForgottenVersion)
{
    // Save as Full_v0 (wire version index = 0)
    Full_v0 original;
    original.a = 99;
    const auto buf = ssSave(original);

    // Try loading as Trunc_v2 (offset=2, doesn't know v0)
    Trunc_v2 loaded;
    EXPECT_THROW(ssLoad(buf, loaded), VersionError);
}

// ============================================================
// Full history reader can read offset-writer data
// ============================================================

TEST(SuitableStruct, Offset_FullHistoryReadsOffsetData)
{
    // Save with truncated history (offset=2)
    Trunc_v3 original;
    original.a = 5;
    original.b = 6.0f;
    original.c = "x";
    original.d = 7;

    const auto buf = ssSave(original);

    // Full history reader (offset=0) loads segment with wire version 3
    Full_v3 loaded;
    ssLoad(buf, loaded);
    EXPECT_EQ(loaded.a, 5);
    EXPECT_FLOAT_EQ(loaded.b, 6.0f);
    EXPECT_EQ(loaded.c, "x");
    EXPECT_EQ(loaded.d, 7);

    // Full history reader loads segment with wire version 2, upgrades to v3
    Full_v2 loaded2;
    ssLoad(buf, loaded2);
    EXPECT_EQ(loaded2.a, 5);
    EXPECT_FLOAT_EQ(loaded2.b, 6.0f);
    EXPECT_EQ(loaded2.c, "x");
}

// ============================================================
// Offset-reader can read full-history data
// ============================================================

TEST(SuitableStruct, Offset_OffsetReadsFullData)
{
    // Save with full history (offset=0, segments: v3, v2, v1, v0)
    Full_v3 original;
    original.a = 1;
    original.b = 2.0f;
    original.c = "y";
    original.d = 3;

    const auto buf = ssSave(original);

    // Truncated reader (offset=2) reads — finds segment with wire version 3
    Trunc_v3 loaded;
    ssLoad(buf, loaded);
    EXPECT_EQ(loaded.a, 1);
    EXPECT_FLOAT_EQ(loaded.b, 2.0f);
    EXPECT_EQ(loaded.c, "y");
    EXPECT_EQ(loaded.d, 3);

    // Truncated reader (offset=2) reads — finds segment with wire version 2, exact match
    Trunc_v2 loaded2;
    ssLoad(buf, loaded2);
    EXPECT_EQ(loaded2.a, 1);
    EXPECT_FLOAT_EQ(loaded2.b, 2.0f);
    EXPECT_EQ(loaded2.c, "y");
}

// ============================================================
// Nested: outer has offset=0, inner has offset=2
// ============================================================

namespace {

struct OuterNoOffset {
    Trunc_v3 inner;
    int extra{};
    using ssVersions = std::tuple<OuterNoOffset>;
    auto ssTuple() const { return std::tie(inner, extra); }
    auto ssNamesTuple() const { return std::tie("inner", "extra"); }
    SS_COMPARISONS_MEMBER(OuterNoOffset)
};

} // namespace

TEST(SuitableStruct, Offset_Nested)
{
    OuterNoOffset original;
    original.inner.a = 11;
    original.inner.b = 22.0f;
    original.inner.c = "nested";
    original.inner.d = 33;
    original.extra = 44;

    const auto buf = ssSave(original);

    OuterNoOffset loaded;
    ssLoad(buf, loaded);
    EXPECT_EQ(loaded, original);
}

// ============================================================
// ssVersionOffset via Handlers
// ============================================================

namespace {

struct HandlersOffset_v2 {
    int a{};
    float b{};
    auto ssTuple() const { return std::tie(a, b); }
    auto ssNamesTuple() const { return std::tie("a", "b"); }
    SS_COMPARISONS_MEMBER(HandlersOffset_v2)
};

} // namespace

template<>
struct SuitableStruct::Handlers<HandlersOffset_v2> : std::true_type {
    using ssVersions = std::tuple<HandlersOffset_v2>;
    static constexpr uint8_t ssVersionOffset = 2;

    static Buffer ssSaveImpl(const HandlersOffset_v2& obj) {
        Buffer buf;
        buf += SuitableStruct::ssSaveInternal(obj.a);
        buf += SuitableStruct::ssSaveInternal(obj.b);
        return buf;
    }

    static void ssLoadImpl(BufferReader& reader, HandlersOffset_v2& obj) {
        SuitableStruct::ssLoadInternal(reader, obj.a);
        SuitableStruct::ssLoadInternal(reader, obj.b);
    }
};

TEST(SuitableStruct, Offset_InHandlers)
{
    EXPECT_EQ(SSVersionOffset<HandlersOffset_v2>::value, 2u);
    EXPECT_EQ(SSVersion<HandlersOffset_v2>::value, 2u); // offset(2) + tuplePos(0)

    HandlersOffset_v2 original;
    original.a = 77;
    original.b = 8.8f;

    const auto buf = ssSave(original);

    // Wire version should be 2
    EXPECT_EQ(firstSegmentWireVersion(buf), 2);

    HandlersOffset_v2 loaded;
    ssLoad(buf, loaded);
    EXPECT_EQ(loaded.a, 77);
    EXPECT_FLOAT_EQ(loaded.b, 8.8f);
}

// ============================================================
// Combined: ssVersionOffset + ssDowngradeTo = delete
// ============================================================

namespace {

struct Combined_v2 {
    int a{};
    float b{};
    using ssVersions = std::tuple<Combined_v2>;
    static constexpr uint8_t ssVersionOffset = 2;
    auto ssTuple() const { return std::tie(a, b); }
    auto ssNamesTuple() const { return std::tie("a", "b"); }
    SS_COMPARISONS_MEMBER(Combined_v2)
};

struct Combined_v3 {
    int a{};
    float b{};
    std::string c;
    using ssVersions = std::tuple<Combined_v2, Combined_v3>;
    static constexpr uint8_t ssVersionOffset = 2;
    auto ssTuple() const { return std::tie(a, b, c); }
    auto ssNamesTuple() const { return std::tie("a", "b", "c"); }
    SS_COMPARISONS_MEMBER(Combined_v3)

    void ssUpgradeFrom(const Combined_v2& prev) { a = prev.a; b = prev.b; c = "combined"; }
    void ssDowngradeTo(Combined_v2&) const = delete; // Only v3 segment written
};

} // namespace

TEST(SuitableStruct, Offset_CombinedWithDowngradeStop)
{
    Combined_v3 original;
    original.a = 50;
    original.b = 5.5f;
    original.c = "combo";

    const auto buf = ssSave(original);

    // ssDowngradeTo = delete → only 1 segment, wire version = 3 (offset=2, tuplePos=1)
    EXPECT_EQ(countBinarySegments(buf), 1);
    auto versions = allSegmentWireVersions(buf);
    ASSERT_EQ(versions.size(), 1u);
    EXPECT_EQ(versions[0], 3);

    // Load as v3 — works
    Combined_v3 loaded3;
    ssLoad(buf, loaded3);
    EXPECT_EQ(loaded3, original);

    // Load as v2 — VersionError (only v3 segment, no downgrade)
    Combined_v2 loaded2;
    EXPECT_THROW(ssLoad(buf, loaded2), VersionError);
}

// ============================================================
// JSON tests
// ============================================================

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <SuitableStruct/SerializerJson.h>

TEST(SuitableStruct, Offset_Json_Basic)
{
    Trunc_v3 original;
    original.a = 42;
    original.b = 3.14f;
    original.c = "test";
    original.d = 99;

    const auto json = ssJsonSave(original);

    Trunc_v3 loaded;
    ssJsonLoad(json, loaded);
    EXPECT_EQ(loaded, original);
}

TEST(SuitableStruct, Offset_Json_Upgrade)
{
    Trunc_v2 original;
    original.a = 10;
    original.b = 2.5f;
    original.c = "old";

    const auto json = ssJsonSave(original);

    Trunc_v3 loaded;
    ssJsonLoad(json, loaded);
    EXPECT_EQ(loaded.a, 10);
    EXPECT_FLOAT_EQ(loaded.b, 2.5f);
    EXPECT_EQ(loaded.c, "old");
    EXPECT_EQ(loaded.d, 42);
}

TEST(SuitableStruct, Offset_Json_ForgottenVersion)
{
    Full_v0 original;
    original.a = 99;

    const auto json = ssJsonSave(original);

    Trunc_v2 loaded;
    EXPECT_THROW(ssJsonLoad(json, loaded), VersionError);
}

TEST(SuitableStruct, Offset_Json_CrossCompat)
{
    // Full writer → truncated reader
    Full_v3 fullOriginal;
    fullOriginal.a = 1;
    fullOriginal.b = 2.0f;
    fullOriginal.c = "y";
    fullOriginal.d = 3;

    const auto json = ssJsonSave(fullOriginal);

    Trunc_v3 truncLoaded;
    ssJsonLoad(json, truncLoaded);
    EXPECT_EQ(truncLoaded.a, 1);
    EXPECT_FLOAT_EQ(truncLoaded.b, 2.0f);
    EXPECT_EQ(truncLoaded.c, "y");
    EXPECT_EQ(truncLoaded.d, 3);

    // Truncated writer → full reader
    Trunc_v3 truncOriginal;
    truncOriginal.a = 5;
    truncOriginal.b = 6.0f;
    truncOriginal.c = "x";
    truncOriginal.d = 7;

    const auto json2 = ssJsonSave(truncOriginal);

    Full_v3 fullLoaded;
    ssJsonLoad(json2, fullLoaded);
    EXPECT_EQ(fullLoaded.a, 5);
    EXPECT_FLOAT_EQ(fullLoaded.b, 6.0f);
    EXPECT_EQ(fullLoaded.c, "x");
    EXPECT_EQ(fullLoaded.d, 7);
}

TEST(SuitableStruct, Offset_Json_CombinedWithDowngradeStop)
{
    Combined_v3 original;
    original.a = 50;
    original.b = 5.5f;
    original.c = "combo";

    const auto json = ssJsonSave(original);

    Combined_v3 loaded3;
    ssJsonLoad(json, loaded3);
    EXPECT_EQ(loaded3, original);

    Combined_v2 loaded2;
    EXPECT_THROW(ssJsonLoad(json, loaded2), VersionError);
}

#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
