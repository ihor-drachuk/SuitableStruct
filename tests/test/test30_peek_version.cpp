/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QByteArray>
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

using namespace SuitableStruct;

namespace {

struct PV_v0 {
    int a {};
    using ssVersions = std::tuple<PV_v0>;
    auto ssTuple() const { return std::tie(a); }
    auto ssNamesTuple() const { return std::tie("a"); }
    SS_COMPARISONS_MEMBER(PV_v0)
};

struct PV_v1 {
    int a {};
    float b {};
    using ssVersions = std::tuple<PV_v0, PV_v1>;
    auto ssTuple() const { return std::tie(a, b); }
    auto ssNamesTuple() const { return std::tie("a", "b"); }
    SS_COMPARISONS_MEMBER(PV_v1)

    void ssUpgradeFrom(const PV_v0& prev) { a = prev.a; b = 1.5f; }
    void ssDowngradeTo(PV_v0& next) const { next.a = a; }
};

struct PV_v2 {
    int a {};
    float b {};
    std::string c;
    using ssVersions = std::tuple<PV_v0, PV_v1, PV_v2>;
    auto ssTuple() const { return std::tie(a, b, c); }
    auto ssNamesTuple() const { return std::tie("a", "b", "c"); }
    SS_COMPARISONS_MEMBER(PV_v2)

    void ssUpgradeFrom(const PV_v1& prev) { a = prev.a; b = prev.b; c = "default"; }
    void ssDowngradeTo(PV_v1& next) const { next.a = a; next.b = b; }
};

struct PV_v3 {
    int a {};
    float b {};
    std::string c;
    int d {};
    using ssVersions = std::tuple<PV_v0, PV_v1, PV_v2, PV_v3>;
    auto ssTuple() const { return std::tie(a, b, c, d); }
    auto ssNamesTuple() const { return std::tie("a", "b", "c", "d"); }
    SS_COMPARISONS_MEMBER(PV_v3)

    void ssUpgradeFrom(const PV_v2& prev) { a = prev.a; b = prev.b; c = prev.c; d = 42; }
    void ssDowngradeTo(PV_v2& next) const { next.a = a; next.b = b; next.c = c; }
};

struct PV_Trunc_v2 {
    int a {};
    float b {};
    std::string c;
    using ssVersions = std::tuple<PV_Trunc_v2>;
    static constexpr uint8_t ssVersionOffset = 2;
    auto ssTuple() const { return std::tie(a, b, c); }
    auto ssNamesTuple() const { return std::tie("a", "b", "c"); }
    SS_COMPARISONS_MEMBER(PV_Trunc_v2)
};

struct PV_Trunc_v3 {
    int a {};
    float b {};
    std::string c;
    int d {};
    using ssVersions = std::tuple<PV_Trunc_v2, PV_Trunc_v3>;
    static constexpr uint8_t ssVersionOffset = 2;
    auto ssTuple() const { return std::tie(a, b, c, d); }
    auto ssNamesTuple() const { return std::tie("a", "b", "c", "d"); }
    SS_COMPARISONS_MEMBER(PV_Trunc_v3)

    void ssUpgradeFrom(const PV_Trunc_v2& prev) { a = prev.a; b = prev.b; c = prev.c; d = 42; }
    void ssDowngradeTo(PV_Trunc_v2& next) const { next.a = a; next.b = b; next.c = c; }
};

Buffer wrapAsProtected(const Buffer& payload)
{
    Buffer result;
    result.write(static_cast<uint64_t>(payload.size()));
    result.write(payload.hash());
    result += payload;
    return result;
}

Buffer buildF1WithSegmentVersions(const std::vector<uint8_t>& versions)
{
    Buffer payload;
    payload.writeRaw(static_cast<const void*>(Internal::SS_FORMAT_F1), sizeof(Internal::SS_FORMAT_F1));
    payload.write(static_cast<uint8_t>(versions.size()));
    for (auto v : versions) {
        payload.write(v);
        payload.write(static_cast<uint64_t>(0));
    }
    return wrapAsProtected(payload);
}

} // namespace

TEST(SuitableStruct, PeekVersion_SingleVersionOffset0)
{
    const PV_v0 obj { 7 };
    const auto buf = ssSave(obj);

    const auto optVersion = ssPeekVersion(buf);
    ASSERT_TRUE(optVersion.has_value());
    EXPECT_EQ(*optVersion, 0u);
    EXPECT_EQ(*optVersion, SSVersion<PV_v0>::value);
}

TEST(SuitableStruct, PeekVersion_FourVersionsOffset0)
{
    PV_v3 obj;
    obj.a = 1;
    obj.b = 2.5f;
    obj.c = "hello";
    obj.d = 99;
    const auto buf = ssSave(obj);

    const auto optVersion = ssPeekVersion(buf);
    ASSERT_TRUE(optVersion.has_value());
    EXPECT_EQ(*optVersion, 3u);
    EXPECT_EQ(*optVersion, SSVersion<PV_v3>::value);
}

TEST(SuitableStruct, PeekVersion_TruncatedSingleVersionOffset2)
{
    PV_Trunc_v2 obj;
    obj.a = 10;
    obj.b = 1.5f;
    obj.c = "x";
    const auto buf = ssSave(obj);

    const auto optVersion = ssPeekVersion(buf);
    ASSERT_TRUE(optVersion.has_value());
    EXPECT_EQ(*optVersion, 2u);
    EXPECT_EQ(*optVersion, SSVersion<PV_Trunc_v2>::value);
}

TEST(SuitableStruct, PeekVersion_TruncatedTwoVersionsOffset2)
{
    PV_Trunc_v3 obj;
    obj.a = 42;
    obj.b = 3.14f;
    obj.c = "test";
    obj.d = 99;
    const auto buf = ssSave(obj);

    const auto optVersion = ssPeekVersion(buf);
    ASSERT_TRUE(optVersion.has_value());
    EXPECT_EQ(*optVersion, 3u);
    EXPECT_EQ(*optVersion, SSVersion<PV_Trunc_v3>::value);
}

TEST(SuitableStruct, PeekVersion_F0BufferReturnsNullopt)
{
    Buffer payload;
    payload.writeRaw(static_cast<const void*>(Internal::SS_FORMAT_F0), sizeof(Internal::SS_FORMAT_F0));
    payload.write(static_cast<uint8_t>(0));
    const auto buf = wrapAsProtected(payload);

    ASSERT_EQ(ssDetectFormat(buf), SSDataFormat::F0);
    EXPECT_FALSE(ssPeekVersion(buf).has_value());
}

TEST(SuitableStruct, PeekVersion_CorruptedHashReturnsNullopt)
{
    PV_v3 obj;
    obj.a = 1;
    auto buf = ssSave(obj);

    // Flip a byte past the format marker so hash mismatches but marker stays valid F1.
    const size_t flipOffset = sizeof(uint64_t) + sizeof(uint32_t) + Internal::SS_FORMAT_MARK_SIZE;
    ASSERT_GT(buf.size(), flipOffset);
    buf.data()[flipOffset] ^= 0xFF;

    EXPECT_FALSE(ssPeekVersion(buf).has_value());
}

TEST(SuitableStruct, PeekVersion_OversizedOuterSizeReturnsNullopt)
{
    PV_v3 obj;
    obj.a = 1;
    const auto good = ssSave(obj);

    Buffer bad;
    const uint64_t realPayloadSize = good.size() - sizeof(uint64_t) - sizeof(uint32_t);
    bad.write(static_cast<uint64_t>(realPayloadSize + 1));
    uint32_t origHash {};
    std::memcpy(&origHash, good.data() + sizeof(uint64_t), sizeof(uint32_t));
    bad.write(origHash);
    bad.writeRaw(good.data() + sizeof(uint64_t) + sizeof(uint32_t), realPayloadSize);

    EXPECT_FALSE(ssPeekVersion(bad).has_value());
}

TEST(SuitableStruct, PeekVersion_OversizedSegmentSizeReturnsNullopt)
{
    Buffer payload;
    payload.writeRaw(static_cast<const void*>(Internal::SS_FORMAT_F1), sizeof(Internal::SS_FORMAT_F1));
    payload.write(static_cast<uint8_t>(1));    // 1 segment
    payload.write(static_cast<uint8_t>(0));    // storedVersion
    payload.write(static_cast<uint64_t>(0xFFFFFFFFFFFFFFFFull)); // bogus huge segmentSize
    const auto buf = wrapAsProtected(payload);

    ASSERT_EQ(ssDetectFormat(buf), SSDataFormat::F1);
    EXPECT_FALSE(ssPeekVersion(buf).has_value());
}

TEST(SuitableStruct, PeekVersion_TooShortBufferReturnsNullopt)
{
    const uint8_t tiny[] = { 0x01, 0x02 };
    const Buffer buf(tiny, sizeof(tiny));
    EXPECT_FALSE(ssPeekVersion(buf).has_value());

    const Buffer empty;
    EXPECT_FALSE(ssPeekVersion(empty).has_value());
}

TEST(SuitableStruct, PeekVersion_UnknownFormatMarkerReturnsNullopt)
{
    Buffer payload;
    const uint8_t bogusMarker[Internal::SS_FORMAT_MARK_SIZE] = { 0x02, 0x00, 0x00, 0x00, 0x00 };
    payload.writeRaw(static_cast<const void*>(bogusMarker), sizeof(bogusMarker));
    payload.write(static_cast<uint8_t>(0));
    const auto buf = wrapAsProtected(payload);

    EXPECT_FALSE(ssDetectFormat(buf).has_value());
    EXPECT_FALSE(ssPeekVersion(buf).has_value());
}

TEST(SuitableStruct, PeekVersion_MaxAmongTwoSegmentsAscending)
{
    const auto buf = buildF1WithSegmentVersions({ 1, 3 });

    ASSERT_EQ(ssDetectFormat(buf), SSDataFormat::F1);

    const auto optVersion = ssPeekVersion(buf);
    ASSERT_TRUE(optVersion.has_value());
    EXPECT_EQ(*optVersion, 3u);
}

TEST(SuitableStruct, PeekVersion_MaxInMiddleOfThreeSegments)
{
    const auto buf = buildF1WithSegmentVersions({ 2, 5, 1 });

    ASSERT_EQ(ssDetectFormat(buf), SSDataFormat::F1);

    const auto optVersion = ssPeekVersion(buf);
    ASSERT_TRUE(optVersion.has_value());
    EXPECT_EQ(*optVersion, 5u);
}

TEST(SuitableStruct, PeekVersion_MaxUint8SegmentCount)
{
    std::vector<uint8_t> versions(255, 0);
    versions[100] = 7;
    const auto buf = buildF1WithSegmentVersions(versions);

    ASSERT_EQ(ssDetectFormat(buf), SSDataFormat::F1);

    const auto optVersion = ssPeekVersion(buf);
    ASSERT_TRUE(optVersion.has_value());
    EXPECT_EQ(*optVersion, 7u);
}

TEST(SuitableStruct, PeekVersion_ZeroSegmentsReturnsNullopt)
{
    const auto buf = buildF1WithSegmentVersions({});

    ASSERT_EQ(ssDetectFormat(buf), SSDataFormat::F1);
    EXPECT_FALSE(ssPeekVersion(buf).has_value());
}

TEST(SuitableStruct, PeekVersion_BufferReaderPositionPreservedOnSuccess)
{
    PV_v3 obj;
    obj.a = 1;
    const auto wholeBuf = ssSave(obj);

    BufferReader reader(wholeBuf);
    const auto originalPosition = reader.position();

    const auto optVersion = ssPeekVersion(reader);
    ASSERT_TRUE(optVersion.has_value());
    EXPECT_EQ(*optVersion, 3u);
    EXPECT_EQ(reader.position(), originalPosition);
}

TEST(SuitableStruct, PeekVersion_BufferReaderPositionPreservedFromNonZeroStart)
{
    Buffer prefix;
    const uint8_t pad[] = { 0xAA, 0xBB, 0xCC, 0xDD };
    prefix.writeRaw(pad, sizeof(pad));

    PV_v3 obj;
    obj.a = 1;
    const auto payload = ssSave(obj);

    Buffer composite;
    composite += prefix;
    composite += payload;

    BufferReader reader(composite);
    reader.advance(static_cast<std::ptrdiff_t>(prefix.size()));
    const auto originalPosition = reader.position();
    ASSERT_EQ(originalPosition, prefix.size());

    const auto optVersion = ssPeekVersion(reader);
    ASSERT_TRUE(optVersion.has_value());
    EXPECT_EQ(*optVersion, 3u);
    EXPECT_EQ(reader.position(), originalPosition);
}

TEST(SuitableStruct, PeekVersion_BufferReaderPositionPreservedOnEarlyReturnPaths)
{
    const uint8_t pad[] = { 0xAA, 0xBB, 0xCC, 0xDD };

    auto checkWithNonZeroStart = [&](const Buffer& payloadBuf) {
        Buffer composite;
        composite.writeRaw(pad, sizeof(pad));
        composite += payloadBuf;

        BufferReader reader(composite);
        reader.advance(static_cast<std::ptrdiff_t>(sizeof(pad)));
        const auto originalPosition = reader.position();
        ASSERT_EQ(originalPosition, sizeof(pad));

        const auto optVersion = ssPeekVersion(reader);
        EXPECT_FALSE(optVersion.has_value());
        EXPECT_EQ(reader.position(), originalPosition);
    };

    {
        Buffer payload;
        payload.writeRaw(static_cast<const void*>(Internal::SS_FORMAT_F0), sizeof(Internal::SS_FORMAT_F0));
        payload.write(static_cast<uint8_t>(0));
        checkWithNonZeroStart(wrapAsProtected(payload));
    }

    checkWithNonZeroStart(buildF1WithSegmentVersions({}));

    {
        PV_v3 obj;
        obj.a = 1;
        auto buf = ssSave(obj);
        const size_t flipOffset = sizeof(uint64_t) + sizeof(uint32_t) + Internal::SS_FORMAT_MARK_SIZE;
        ASSERT_GT(buf.size(), flipOffset);
        buf.data()[flipOffset] ^= 0xFF;
        checkWithNonZeroStart(buf);
    }

    {
        PV_v3 obj;
        obj.a = 1;
        const auto wholeBuf = ssSave(obj);
        BufferReader reader(wholeBuf);
        reader.advance(3);
        const auto originalPosition = reader.position();
        const auto optVersion = ssPeekVersion(reader);
        EXPECT_FALSE(optVersion.has_value());
        EXPECT_EQ(reader.position(), originalPosition);
    }
}

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
TEST(SuitableStruct, PeekVersion_QByteArrayOverload)
{
    PV_v3 obj;
    obj.a = 7;
    obj.b = 1.5f;
    obj.c = "qba";
    obj.d = 11;
    const auto buf = ssSave(obj);
    const QByteArray qba = buf.toQByteArray();

    ASSERT_EQ(ssDetectFormat(qba), SSDataFormat::F1);

    const auto optVersion = ssPeekVersion(qba);
    ASSERT_TRUE(optVersion.has_value());
    EXPECT_EQ(*optVersion, 3u);

    QByteArray corrupted = qba;
    const int flipOffset = static_cast<int>(sizeof(uint64_t) + sizeof(uint32_t) + Internal::SS_FORMAT_MARK_SIZE);
    ASSERT_GT(corrupted.size(), flipOffset);
    corrupted[flipOffset] = corrupted[flipOffset] ^ char(0xFF);

    EXPECT_FALSE(ssDetectFormat(corrupted).has_value());
    EXPECT_FALSE(ssPeekVersion(corrupted).has_value());
}
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
