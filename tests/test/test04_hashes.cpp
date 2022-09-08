#include <gtest/gtest.h>
#include <string>
#include <unordered_set>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Hashes.h>

using namespace SuitableStruct;

namespace {

struct SomeStruct1
{
    int a {};
    std::string b;

    auto ssTuple() const { return std::tie(a, b); }

    // For test
    bool operator==(const SomeStruct1& rhs) const { return (ssTuple() == rhs.ssTuple()); }
};

struct SomeStruct2
{
    SomeStruct1 struct1;
    float c {};
    std::shared_ptr<int> d;

    auto ssTuple() const { return std::tie(struct1, c, d); }

    // For test
    bool operator==(const SomeStruct2& rhs) const { return (ssTuple() == rhs.ssTuple()); }
};

} // namespace

SS_HASHES(SomeStruct2); // Enable std::hash for it.
                        // E.g. enable use in std::unordered_set.


TEST(SuitableStruct, HashesTest)
{
    SomeStruct2 s2;
    auto h1 = ssHash(s2);
    auto h2 = ssHash(s2);
    ASSERT_EQ(h1, h2);

    ssHash(std::make_shared<int>(15));

    std::unordered_set<SomeStruct2> set;
    set.insert(s2);
}
