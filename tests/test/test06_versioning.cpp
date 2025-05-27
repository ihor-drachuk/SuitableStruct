/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Exceptions.h>
#include <stdexcept>

using namespace SuitableStruct;

namespace {
struct Struct_v0
{
    int a {};
    //using ssVersions = std::tuple<Struct_v0>;
};
} // namespace


namespace SuitableStruct {
template<>
struct Handlers<Struct_v0> : public std::true_type
{
    static Buffer ssSaveImpl(const Struct_v0& v) { return Buffer::fromValue(v.a); }
    static void ssLoadImpl(BufferReader& bufferReader, Struct_v0& value) { bufferReader.read(value.a); }
    static uint32_t ssHashImpl(const Struct_v0& v) { return v.a; }
};
} // namespace SuitableStruct


namespace {
struct Struct_v1
{
    int a {};
    float b {};

    auto ssTuple() const { return std::tie(a, b); }

    using ssVersions = std::tuple<Struct_v0, Struct_v1>;
    void ssUpgradeFrom(const Struct_v0& prev) { a = prev.a; b = 0; }
    void ssDowngradeTo(Struct_v0& next) const { next.a = a; }
    SS_COMPARISONS_MEMBER(Struct_v1)
};


struct Struct_v2
{
    int a {};
    double b {};
    std::string c;

    using ssVersions = std::tuple<Struct_v0, Struct_v1, Struct_v2>;
    auto ssTuple() const { return std::tie(a, b, c); }
    void ssUpgradeFrom(const Struct_v1& prev) { a = prev.a; b = prev.b; c = "Q"; }
    void ssDowngradeTo(Struct_v1& next) const { next.a = a; next.b = static_cast<float>(b); }
    SS_COMPARISONS_MEMBER(Struct_v2)
};


using Struct = Struct_v2;

struct NestedTest_Old_Helper
{
    Struct_v0 s;
    auto ssTuple() const { return std::tie(s); }
};

struct NestedTest
{
    Struct s;

    auto ssTuple() const { return std::tie(s); }
    SS_COMPARISONS_MEMBER(NestedTest)
};

struct CastableStruct_v0
{
    int value {};

    using ssVersions = std::tuple<CastableStruct_v0>;
    auto ssTuple() const { return std::tie(value); }
    SS_COMPARISONS_MEMBER(CastableStruct_v0)
};

struct CastableStruct_v1
{
    int value {};
    float extra {};

    using ssVersions = std::tuple<CastableStruct_v0, CastableStruct_v1>;
    auto ssTuple() const { return std::tie(value, extra); }

    // Explicit conversion constructor for C++ casting fallback
    CastableStruct_v1(const CastableStruct_v0& old) : value(old.value), extra(0.0f) {}
    CastableStruct_v1() = default;

    SS_COMPARISONS_MEMBER(CastableStruct_v1)
};

// Exception-throwing test structs
struct ExceptionStruct_v0
{
    int value {};

    using ssVersions = std::tuple<ExceptionStruct_v0>;
    auto ssTuple() const { return std::tie(value); }
    SS_COMPARISONS_MEMBER(ExceptionStruct_v0)
};

struct ExceptionStruct_v1
{
    int value {};
    float extra {};

    using ssVersions = std::tuple<ExceptionStruct_v0, ExceptionStruct_v1>;
    auto ssTuple() const { return std::tie(value, extra); }

    void ssUpgradeFrom(const ExceptionStruct_v0& /*prev*/) {
        throw std::runtime_error("Upgrade failed!");
    }

    void ssDowngradeTo(ExceptionStruct_v0& /*next*/) const {
        throw std::runtime_error("Downgrade failed!");
    }

    SS_COMPARISONS_MEMBER(ExceptionStruct_v1)
};
} // namespace


TEST(SuitableStruct, VersioningTest_0_to_1)
{
    Struct_v0 v0;
    v0.a = 17;

    const auto buf = ssSave(v0);

    Struct_v1 v1;
    ssLoad(buf, v1);

    ASSERT_EQ(v1.a, v0.a);
    ASSERT_DOUBLE_EQ(v1.b, 0);
}

TEST(SuitableStruct, VersioningTest_1_to_2)
{
    Struct_v1 v1;
    v1.a = 17;
    v1.b = 1.7f;

    const auto buf = ssSave(v1);

    Struct_v2 v2;
    ssLoad(buf, v2);

    ASSERT_EQ(v2.a, v1.a);
    ASSERT_DOUBLE_EQ(v2.b, v1.b);
    ASSERT_EQ(v2.c, "Q");
}

TEST(SuitableStruct, VersioningTest_0_to_2)
{
    Struct_v0 v0;
    v0.a = 17;

    const auto buf = ssSave(v0);

    Struct_v2 v2;
    ssLoad(buf, v2);

    ASSERT_EQ(v2.a, v0.a);
    ASSERT_DOUBLE_EQ(v2.b, 0);
    ASSERT_EQ(v2.c, "Q");
}

TEST(SuitableStruct, VersioningTest_0_to_0)
{
    Struct_v0 v0;
    v0.a = 17;

    const auto buf = ssSave(v0);

    Struct_v0 v0r;
    ssLoad(buf, v0r);

    ASSERT_EQ(v0r.a, v0.a);
}

TEST(SuitableStruct, VersioningTest_1_to_1)
{
    Struct_v1 v1;
    v1.a = 17;
    v1.b = 1.7f;

    const auto buf = ssSave(v1);

    Struct_v1 v1r;
    ssLoad(buf, v1r);

    ASSERT_EQ(v1r, v1);
}

TEST(SuitableStruct, VersioningTest_2_to_2)
{
    Struct_v2 v2;
    v2.a = 17;
    v2.b = 1.7;
    v2.c = "W";

    const auto buf = ssSave(v2);

    Struct_v2 v2r;
    ssLoad(buf, v2r);

    ASSERT_EQ(v2r, v2);
}

TEST(SuitableStruct, VersioningTest_2_to_1)
{
    Struct_v2 v2;
    v2.a = 17;
    v2.b = 1.7;
    v2.c = "W";

    const auto buf = ssSave(v2);

    Struct_v1 v1;
    ssLoad(buf, v1);

    ASSERT_EQ(v1.a, v2.a);
    ASSERT_FLOAT_EQ(v1.b, static_cast<float>(v2.b));
}

TEST(SuitableStruct, VersioningTest_2_to_0)
{
    Struct_v2 v2;
    v2.a = 17;
    v2.b = 1.7;
    v2.c = "W";

    const auto buf = ssSave(v2);

    Struct_v0 v0;
    ssLoad(buf, v0);

    ASSERT_EQ(v0.a, v2.a);
}

TEST(SuitableStruct, VersioningTest_1_to_0)
{
    Struct_v1 v1;
    v1.a = 17;
    v1.b = 1.7f;

    const auto buf = ssSave(v1);

    Struct_v0 v0;
    ssLoad(buf, v0);

    ASSERT_EQ(v0.a, v1.a);
}

TEST(SuitableStruct, VersioningTest_Nested_0_to_2)
{
    NestedTest_Old_Helper v;
    v.s.a = 17;

    const auto buf = ssSave(v);

    NestedTest v2;
    ssLoad(buf, v2);

    NestedTest v2ref;
    v2ref.s = {17, 0, "Q"};

    ASSERT_EQ(v2, v2ref);
}

TEST(SuitableStruct, VersioningTest_Nested_2_to_0)
{
    NestedTest v;
    v.s = {17, 1.7, "W"};

    const auto buf = ssSave(v);

    NestedTest_Old_Helper v0;
    ssLoad(buf, v0);

    ASSERT_EQ(v0.s.a, v.s.a);
}

TEST(SuitableStruct, VersioningTest_UnknownVersion)
{
    Buffer buf;

    // First write the data part
    buf.writeRaw(static_cast<const void*>(Internal::SS_FORMAT_F1), sizeof(Internal::SS_FORMAT_F1));
    buf.write(static_cast<uint8_t>(1)); // segments count
    buf.write(static_cast<uint8_t>(99)); // unknown version
    buf.write(static_cast<uint64_t>(0)); // empty segment size

    // Now create the final buffer with proper format
    Buffer finalBuf;
    finalBuf.write(static_cast<uint64_t>(buf.size())); // total size
    finalBuf.write(buf.hash()); // hash
    finalBuf += buf; // append the data part

    Struct_v0 v0;
    EXPECT_THROW(ssLoad(finalBuf, v0), VersionError);

    Struct_v1 v1;
    EXPECT_THROW(ssLoad(finalBuf, v1), VersionError);

    Struct_v2 v2;
    EXPECT_THROW(ssLoad(finalBuf, v2), VersionError);
}

TEST(SuitableStruct, VersioningTest_InvalidFormat)
{
    // Create a buffer with invalid format signature
    Buffer buf;
    uint8_t invalid_signature[Internal::SS_FORMAT_MARK_SIZE] = { 'I', 'N', 'V', 'A', 'L' };
    buf.writeRaw(static_cast<const void*>(invalid_signature), sizeof(invalid_signature));

    // Now create the final buffer with proper format
    Buffer finalBuf;
    finalBuf.write(static_cast<uint64_t>(buf.size())); // total size
    finalBuf.write(buf.hash()); // hash
    finalBuf += buf; // append the data part

    Struct_v0 v0;
    EXPECT_THROW(ssLoad(finalBuf, v0), FormatError);

    Struct_v1 v1;
    EXPECT_THROW(ssLoad(finalBuf, v1), FormatError);

    Struct_v2 v2;
    EXPECT_THROW(ssLoad(finalBuf, v2), FormatError);
}

TEST(SuitableStruct, VersioningTest_UpgradeCastingFallback)
{
    // Test that C++ casting fallback works for upgrades when ssUpgradeFrom is not defined
    CastableStruct_v0 oldStruct;
    oldStruct.value = 42;

    const auto buf = ssSave(oldStruct);

    CastableStruct_v1 newStruct;
    ssLoad(buf, newStruct);

    // Should have converted using the C++ constructor
    ASSERT_EQ(newStruct.value, 42);
    ASSERT_EQ(newStruct.extra, 0.0f);
}

TEST(SuitableStruct, VersioningTest_ExceptionInUpgrade)
{
    ExceptionStruct_v0 v0;
    v0.value = 42;

    const auto buf = ssSave(v0);

    ExceptionStruct_v1 v1;
    EXPECT_THROW(ssLoad(buf, v1), std::runtime_error);
}

TEST(SuitableStruct, VersioningTest_ExceptionInDowngrade)
{
    ExceptionStruct_v1 v1;
    v1.value = 42;
    v1.extra = 3.14f;

    EXPECT_THROW(ssSave(v1), std::runtime_error);
}
