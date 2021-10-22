#include <gtest/gtest.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>

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
    static void ssLoadImpl(BufferReader& buffer, Struct_v0& value) { buffer.read(value.a); }
    static uint32_t ssHashImpl(const Struct_v0& v) { return v.a; };
};
} // namespace


namespace {
struct Struct_v1
{
    int a {};
    float b {};

    auto ssTuple() const { return std::tie(a, b); }

    using ssVersions = std::tuple<Struct_v0, Struct_v1>;
    void ssConvertFrom(const Struct_v0& prev) { a = prev.a; b = 0; }
};
SS_COMPARISONS(Struct_v1);
} // namespace


namespace {
struct Struct_v2
{
    int a {};
    double b {};
    std::string c;

    using ssVersions = std::tuple<Struct_v0, Struct_v1, Struct_v2>;
    auto ssTuple() const { return std::tie(a, b, c); }
    void ssConvertFrom(const Struct_v1& prev) { a = prev.a; b = prev.b; c = "Q"; }
};

SS_COMPARISONS(Struct_v2);
} // namespace


TEST(SuitableStruct, VersioningTest_0_to_1)
{
    Struct_v0 v0;
    v0.a = 17;

    auto buf = ssSave(v0);

    Struct_v1 v1;
    ssLoad(buf, v1);

    ASSERT_EQ(v1.a, v0.a);
    ASSERT_DOUBLE_EQ(v1.b, 0);
}

TEST(SuitableStruct, VersioningTest_1_to_2)
{
    Struct_v1 v1;
    v1.a = 17;
    v1.b = 1.7;

    auto buf = ssSave(v1);

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

    auto buf = ssSave(v0);

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

    auto buf = ssSave(v0);

    Struct_v0 v0r;
    ssLoad(buf, v0r);

    ASSERT_EQ(v0r.a, v0.a);
}

TEST(SuitableStruct, VersioningTest_1_to_1)
{
    Struct_v1 v1;
    v1.a = 17;
    v1.b = 1.7;

    auto buf = ssSave(v1);

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

    auto buf = ssSave(v2);

    Struct_v2 v2r;
    ssLoad(buf, v2r);

    ASSERT_EQ(v2r, v2);
}
