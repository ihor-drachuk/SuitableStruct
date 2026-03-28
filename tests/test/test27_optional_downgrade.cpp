/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

// Tests for explicit opt-out from downgrade (ssDowngradeTo = delete) and
// VersionError behavior when older reader encounters data without
// a matching version segment.

#include <gtest/gtest.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Exceptions.h>

using namespace SuitableStruct;

namespace {

// ============================================================
// Full downgrade chain: v0 → v1 → v2 (all downgrades implemented)
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

// ============================================================
// Partial chain: v2→v1 has downgrade, v1→v0 downgrade is deleted
// ============================================================

struct Partial_v0 {
    int a{};
    using ssVersions = std::tuple<Partial_v0>;
    auto ssTuple() const { return std::tie(a); }
    auto ssNamesTuple() const { return std::tie("a"); }
    SS_COMPARISONS_MEMBER(Partial_v0)
};

struct Partial_v1 {
    int a{};
    float b{};
    using ssVersions = std::tuple<Partial_v0, Partial_v1>;
    auto ssTuple() const { return std::tie(a, b); }
    auto ssNamesTuple() const { return std::tie("a", "b"); }
    SS_COMPARISONS_MEMBER(Partial_v1)

    void ssUpgradeFrom(const Partial_v0& prev) { a = prev.a; b = 2.0f; }
    void ssDowngradeTo(Partial_v0&) const = delete;  // Explicit: do NOT write v0 segment
};

struct Partial_v2 {
    int a{};
    float b{};
    std::string c;
    using ssVersions = std::tuple<Partial_v0, Partial_v1, Partial_v2>;
    auto ssTuple() const { return std::tie(a, b, c); }
    auto ssNamesTuple() const { return std::tie("a", "b", "c"); }
    SS_COMPARISONS_MEMBER(Partial_v2)

    void ssUpgradeFrom(const Partial_v1& prev) { a = prev.a; b = prev.b; c = "upgraded"; }
    void ssDowngradeTo(Partial_v1& next) const { next.a = a; next.b = b; }
};

// ============================================================
// No downgrade at all: v2 has ssDowngradeTo = delete
// ============================================================

struct NoDown_v0 {
    int a{};
    using ssVersions = std::tuple<NoDown_v0>;
    auto ssTuple() const { return std::tie(a); }
    auto ssNamesTuple() const { return std::tie("a"); }
    SS_COMPARISONS_MEMBER(NoDown_v0)
};

struct NoDown_v1 {
    int a{};
    float b{};
    using ssVersions = std::tuple<NoDown_v0, NoDown_v1>;
    auto ssTuple() const { return std::tie(a, b); }
    auto ssNamesTuple() const { return std::tie("a", "b"); }
    SS_COMPARISONS_MEMBER(NoDown_v1)

    void ssUpgradeFrom(const NoDown_v0& prev) { a = prev.a; b = 3.0f; }
    // No downgrade, but that's OK because v2 will stop before reaching v1→v0
};

struct NoDown_v2 {
    int a{};
    float b{};
    std::string c;
    using ssVersions = std::tuple<NoDown_v0, NoDown_v1, NoDown_v2>;
    auto ssTuple() const { return std::tie(a, b, c); }
    auto ssNamesTuple() const { return std::tie("a", "b", "c"); }
    SS_COMPARISONS_MEMBER(NoDown_v2)

    void ssUpgradeFrom(const NoDown_v1& prev) { a = prev.a; b = prev.b; c = "v2only"; }
    void ssDowngradeTo(NoDown_v1&) const = delete;  // Only v2 segment written
};

// ============================================================
// Handlers-based serialization with deleted downgrade on type
// ============================================================

struct HandlersStop_v0 {
    int a{};
    using ssVersions = std::tuple<HandlersStop_v0>;
    auto ssTuple() const { return std::tie(a); }
    auto ssNamesTuple() const { return std::tie("a"); }
    SS_COMPARISONS_MEMBER(HandlersStop_v0)
};

struct HandlersStop_v1 {
    int a{};
    float b{};
    using ssVersions = std::tuple<HandlersStop_v0, HandlersStop_v1>;
    auto ssTuple() const { return std::tie(a, b); }
    auto ssNamesTuple() const { return std::tie("a", "b"); }
    SS_COMPARISONS_MEMBER(HandlersStop_v1)

    void ssUpgradeFrom(const HandlersStop_v0& prev) { a = prev.a; b = 4.0f; }
    void ssDowngradeTo(HandlersStop_v0&) const = delete;
};

} // namespace

// Custom Handlers (serialization via Handlers, downgrade opt-out via = delete on type)
template<>
struct SuitableStruct::Handlers<HandlersStop_v1> : std::true_type {
    static Buffer ssSaveImpl(const HandlersStop_v1& obj) {
        Buffer buf;
        buf += SuitableStruct::ssSaveInternal(obj.a);
        buf += SuitableStruct::ssSaveInternal(obj.b);
        return buf;
    }

    static void ssLoadImpl(BufferReader& reader, HandlersStop_v1& obj) {
        SuitableStruct::ssLoadInternal(reader, obj.a);
        SuitableStruct::ssLoadInternal(reader, obj.b);
    }
};

namespace {

// ============================================================
// Helper: count segments in binary buffer
// ============================================================

uint8_t countBinarySegments(const Buffer& buffer)
{
    BufferReader reader(buffer);

    // Protected mode: size(8) + hash(4) + format_mark(5) + segmentsCount(1) ...
    reader.advance(sizeof(uint64_t)); // size
    reader.advance(sizeof(uint32_t)); // hash
    reader.advance(5);                // format marker
    uint8_t count{};
    reader.read(count);
    return count;
}

} // namespace


// ============================================================
// Tests: Full downgrade chain
// ============================================================

TEST(SuitableStruct, OptDown_FullChain)
{
    Full_v2 original;
    original.a = 42;
    original.b = 3.14f;
    original.c = "hello";

    const auto buf = ssSave(original);

    // Should have 3 segments
    EXPECT_EQ(countBinarySegments(buf), 3);

    // Load as v2
    Full_v2 loaded2;
    ssLoad(buf, loaded2);
    EXPECT_EQ(loaded2, original);

    // Load as v1 (downgrade)
    Full_v1 loaded1;
    ssLoad(buf, loaded1);
    EXPECT_EQ(loaded1.a, 42);
    EXPECT_FLOAT_EQ(loaded1.b, 3.14f);

    // Load as v0 (downgrade)
    Full_v0 loaded0;
    ssLoad(buf, loaded0);
    EXPECT_EQ(loaded0.a, 42);
}

// ============================================================
// Tests: Partial chain (stop at v1)
// ============================================================

TEST(SuitableStruct, OptDown_StopAtV1)
{
    Partial_v2 original;
    original.a = 10;
    original.b = 2.5f;
    original.c = "test";

    const auto buf = ssSave(original);

    // Should have 2 segments (v2, v1), NOT 3
    EXPECT_EQ(countBinarySegments(buf), 2);

    // Load as v2 — works
    Partial_v2 loaded2;
    ssLoad(buf, loaded2);
    EXPECT_EQ(loaded2, original);

    // Load as v1 — works (downgrade from v2)
    Partial_v1 loaded1;
    ssLoad(buf, loaded1);
    EXPECT_EQ(loaded1.a, 10);
    EXPECT_FLOAT_EQ(loaded1.b, 2.5f);

    // Load as v0 — VersionError (no v0 segment)
    Partial_v0 loaded0;
    EXPECT_THROW(ssLoad(buf, loaded0), VersionError);
}

// ============================================================
// Tests: No downgrade (stop at v2)
// ============================================================

TEST(SuitableStruct, OptDown_StopAtV2)
{
    NoDown_v2 original;
    original.a = 99;
    original.b = 1.0f;
    original.c = "only_v2";

    const auto buf = ssSave(original);

    // Should have 1 segment (v2 only)
    EXPECT_EQ(countBinarySegments(buf), 1);

    // Load as v2 — works
    NoDown_v2 loaded2;
    ssLoad(buf, loaded2);
    EXPECT_EQ(loaded2, original);

    // Load as v1 — VersionError
    NoDown_v1 loaded1;
    EXPECT_THROW(ssLoad(buf, loaded1), VersionError);

    // Load as v0 — VersionError
    NoDown_v0 loaded0;
    EXPECT_THROW(ssLoad(buf, loaded0), VersionError);
}

// ============================================================
// Tests: Handlers-based serialization with deleted downgrade
// ============================================================

TEST(SuitableStruct, OptDown_StopInHandlers)
{
    HandlersStop_v1 original;
    original.a = 55;
    original.b = 7.7f;

    const auto buf = ssSave(original);

    // Should have 1 segment (v1 only, stop via Handlers)
    EXPECT_EQ(countBinarySegments(buf), 1);

    // Load as v1 — works
    HandlersStop_v1 loaded1;
    ssLoad(buf, loaded1);
    EXPECT_EQ(loaded1.a, 55);
    EXPECT_FLOAT_EQ(loaded1.b, 7.7f);

    // Load as v0 — VersionError
    HandlersStop_v0 loaded0;
    EXPECT_THROW(ssLoad(buf, loaded0), VersionError);
}

// ============================================================
// Tests: VersionError is catchable (not abort)
// ============================================================

TEST(SuitableStruct, OldReader_Catchable)
{
    NoDown_v2 original;
    original.a = 1;
    original.b = 2.0f;
    original.c = "x";

    const auto buf = ssSave(original);

    bool caught = false;
    try {
        NoDown_v0 loaded;
        ssLoad(buf, loaded);
    } catch (const VersionError&) {
        caught = true;
    } catch (...) {
        FAIL() << "Expected VersionError, got a different exception type";
    }
    EXPECT_TRUE(caught);
}

// ============================================================
// Tests: backward compat — full chain data works with older reader
// ============================================================

TEST(SuitableStruct, OptDown_BackwardCompat)
{
    // Save with full chain (3 segments)
    Full_v2 original;
    original.a = 7;
    original.b = 8.0f;
    original.c = "compat";

    const auto buf = ssSave(original);
    EXPECT_EQ(countBinarySegments(buf), 3);

    // All version readers can load
    Full_v2 v2; ssLoad(buf, v2); EXPECT_EQ(v2.a, 7);
    Full_v1 v1; ssLoad(buf, v1); EXPECT_EQ(v1.a, 7);
    Full_v0 v0; ssLoad(buf, v0); EXPECT_EQ(v0.a, 7);
}

// ============================================================
// Tests: segment count exact verification
// ============================================================

TEST(SuitableStruct, OptDown_SegmentCount)
{
    // Full chain → 3 segments
    {
        Full_v2 obj; obj.a = 1; obj.b = 1.0f; obj.c = "a";
        EXPECT_EQ(countBinarySegments(ssSave(obj)), 3);
    }

    // Partial chain (stop at v1) → 2 segments
    {
        Partial_v2 obj; obj.a = 1; obj.b = 1.0f; obj.c = "a";
        EXPECT_EQ(countBinarySegments(ssSave(obj)), 2);
    }

    // No downgrade (stop at v2) → 1 segment
    {
        NoDown_v2 obj; obj.a = 1; obj.b = 1.0f; obj.c = "a";
        EXPECT_EQ(countBinarySegments(ssSave(obj)), 1);
    }

    // Single version (no versioning) → 1 segment
    {
        Full_v0 obj; obj.a = 1;
        EXPECT_EQ(countBinarySegments(ssSave(obj)), 1);
    }

    // Handlers stop → 1 segment
    {
        HandlersStop_v1 obj; obj.a = 1; obj.b = 1.0f;
        EXPECT_EQ(countBinarySegments(ssSave(obj)), 1);
    }
}

// ============================================================
// Tests: upgrade through deleted-downgrade boundary
// Save as v0, load as v2 (v1 has deleted downgrade, but upgrade still works)
// ============================================================

TEST(SuitableStruct, OptDown_UpgradeThroughDeletedBoundary)
{
    // Save as Partial_v0
    Partial_v0 v0;
    v0.a = 33;
    const auto buf = ssSave(v0);

    // Load as Partial_v2 — should upgrade v0→v1→v2
    // v1 has ssDowngradeTo = delete, but ssUpgradeFrom works fine
    Partial_v2 loaded;
    ssLoad(buf, loaded);
    EXPECT_EQ(loaded.a, 33);
    EXPECT_FLOAT_EQ(loaded.b, 2.0f);  // from v1::ssUpgradeFrom
    EXPECT_EQ(loaded.c, "upgraded");   // from v2::ssUpgradeFrom
}

// ============================================================
// JSON tests
// ============================================================

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <SuitableStruct/SerializerJson.h>

TEST(SuitableStruct, OptDown_Json_FullChain)
{
    Full_v2 original;
    original.a = 42;
    original.b = 3.14f;
    original.c = "hello";

    const auto json = ssJsonSave(original);

    Full_v2 loaded2;
    ssJsonLoad(json, loaded2);
    EXPECT_EQ(loaded2, original);

    Full_v1 loaded1;
    ssJsonLoad(json, loaded1);
    EXPECT_EQ(loaded1.a, 42);
    EXPECT_FLOAT_EQ(loaded1.b, 3.14f);

    Full_v0 loaded0;
    ssJsonLoad(json, loaded0);
    EXPECT_EQ(loaded0.a, 42);
}

TEST(SuitableStruct, OptDown_Json_StopAtV1)
{
    Partial_v2 original;
    original.a = 10;
    original.b = 2.5f;
    original.c = "test";

    const auto json = ssJsonSave(original);

    Partial_v2 loaded2;
    ssJsonLoad(json, loaded2);
    EXPECT_EQ(loaded2, original);

    Partial_v1 loaded1;
    ssJsonLoad(json, loaded1);
    EXPECT_EQ(loaded1.a, 10);
    EXPECT_FLOAT_EQ(loaded1.b, 2.5f);

    Partial_v0 loaded0;
    EXPECT_THROW(ssJsonLoad(json, loaded0), VersionError);
}

TEST(SuitableStruct, OptDown_Json_StopAtV2)
{
    NoDown_v2 original;
    original.a = 99;
    original.b = 1.0f;
    original.c = "only_v2";

    const auto json = ssJsonSave(original);

    NoDown_v2 loaded2;
    ssJsonLoad(json, loaded2);
    EXPECT_EQ(loaded2, original);

    NoDown_v1 loaded1;
    EXPECT_THROW(ssJsonLoad(json, loaded1), VersionError);

    NoDown_v0 loaded0;
    EXPECT_THROW(ssJsonLoad(json, loaded0), VersionError);
}

TEST(SuitableStruct, OptDown_Json_UpgradeThroughDeletedBoundary)
{
    Partial_v0 v0;
    v0.a = 33;
    const auto json = ssJsonSave(v0);

    Partial_v2 loaded;
    ssJsonLoad(json, loaded);
    EXPECT_EQ(loaded.a, 33);
    EXPECT_FLOAT_EQ(loaded.b, 2.0f);
    EXPECT_EQ(loaded.c, "upgraded");
}

#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
