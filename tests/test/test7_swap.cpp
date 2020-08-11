#include <gtest/gtest.h>
#include <SuitableStruct/Swap.h>
#include <SuitableStruct/Comparisons.h>

using namespace SuitableStruct;


struct SomeStruct1
{
    int a {};
    std::string b;

    auto ssTuple() const { return std::tie(a, b); }
};

SS_COMPARISONS(SomeStruct1);
SS_SWAP(SomeStruct1);

struct SomeStruct2
{
    SomeStruct1 struct1;
    float c {};

    auto ssTuple() const { return std::tie(struct1, c); }
};

SS_COMPARISONS(SomeStruct2);
SS_SWAP(SomeStruct2);


TEST(SuitableStruct, SwapTest)
{
    // 1-level
    SomeStruct1 a, b;
    a.a = 1;
    a.b = "ABC";

    b.a = 10;
    b.b = "QWE";

    auto backupA = a;
    auto backupB = b;

    ssSwap(a, b);

    ASSERT_EQ(a, backupB);
    ASSERT_EQ(b, backupA);

    // 2-level
    SomeStruct2 a2, b2;
    a2.struct1 = backupA;
    a2.c = 1.5;

    b2.struct1 = backupB;
    b2.c = 3;

    auto backupA2 = a2;
    auto backupB2 = b2;

    ssSwap(a2, b2);

    ASSERT_EQ(a2, backupB2);
    ASSERT_EQ(b2, backupA2);
}
