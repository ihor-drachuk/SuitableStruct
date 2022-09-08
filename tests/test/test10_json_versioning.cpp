#include <gtest/gtest.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <algorithm>
#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/SerializerJson.h>
#include <SuitableStruct/Containers/vector.h>
#include <string>

using namespace SuitableStruct;

namespace {

struct Struct1
{
    int a{}, b{};

    auto ssTuple() const { return std::tie(a, b); }
    auto ssNamesTuple() const { return std::tie("a", "b"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct1);
};

struct Struct2
{
    std::vector<int> values;

    using ssVersions = std::tuple<Struct1, Struct2>;
    auto ssTuple() const { return std::tie(values); }
    auto ssNamesTuple() const { return std::tie("values"); }
    void ssConvertFrom(const Struct1& prev) { values = {prev.a, prev.b}; }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct2);
};

struct Struct3
{
    std::vector<std::string> values;

    using ssVersions = std::tuple<Struct1, Struct2, Struct3>;
    auto ssTuple() const { return std::tie(values); }
    auto ssNamesTuple() const { return std::tie("values"); }

    void ssConvertFrom(const Struct2& prev) {
        values.clear();

        for (const auto& x : prev.values)
            values.push_back(std::to_string(x));
    }

    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct3);
};

using Struct = Struct3; // Latest version

struct NestedStructs
{
    Struct value;

    auto ssTuple() const { return std::tie(value); }
    auto ssNamesTuple() const { return std::tie("value"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(NestedStructs);
};

struct NestedStructs_Old_Helper_For_Test
{
    Struct1 value;

    auto ssTuple() const { return std::tie(value); }
    auto ssNamesTuple() const { return std::tie("value"); }
};

} // namespace

TEST(SuitableStruct, JsonVersioning)
{
    Struct1 s1;
    s1 = {1,2};
    auto saved1 = ssJsonSave(s1);

    Struct2 s2, s2ref;
    ssJsonLoad(saved1, s2);
    s2ref.values = {1,2};
    ASSERT_EQ(s2, s2ref);
    auto saved2 = ssJsonSave(s2);

    Struct3 s3, s3ref;
    ssJsonLoad(saved2, s3);
    s3ref.values = {"1","2"};
    ASSERT_EQ(s3, s3ref);
}

TEST(SuitableStruct, JsonVersioningNested)
{
    NestedStructs_Old_Helper_For_Test value1;
    value1.value = {1, 2};

    const auto saved = ssJsonSave(value1);

    NestedStructs value2;
    ssJsonLoad(saved, value2);

    NestedStructs value2ref;
    value2ref.value.values = {"1", "2"};

    ASSERT_EQ(value2, value2ref);
}

#endif // #ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
