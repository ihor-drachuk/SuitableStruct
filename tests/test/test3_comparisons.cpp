#include <gtest/gtest.h>
#include <SuitableStruct/Comparisons.h>

namespace {

struct A
{
    int value {};
    auto ssTuple() const { return std::tie(value); }
};

SS_COMPARISONS(A);


struct B
{
    A a;
    int value2 {};
    float value3 {};

    auto ssTuple() const { return std::tie(a, value2, value3); }
};

SS_COMPARISONS(B);

} // namespace


TEST(SuitableStruct, ComparisonsTest)
{
    B b1, b2;

    // b1 == b2
    ASSERT_TRUE  (b1 == b2);
    ASSERT_FALSE (b1 != b2);
    ASSERT_FALSE (b1 <  b2);
    ASSERT_TRUE  (b1 <= b2);
    ASSERT_FALSE (b1 >  b2);
    ASSERT_TRUE  (b1 >= b2);

    // b1 < b2
    b2.a.value = 1;

    ASSERT_FALSE (b1 == b2);
    ASSERT_TRUE  (b1 != b2);
    ASSERT_TRUE  (b1 <  b2);
    ASSERT_TRUE  (b1 <= b2);
    ASSERT_FALSE (b1 >  b2);
    ASSERT_FALSE (b1 >= b2);

    // b1 > b2
    b1.a.value = 2;

    ASSERT_FALSE (b1 == b2);
    ASSERT_TRUE  (b1 != b2);
    ASSERT_FALSE (b1 <  b2);
    ASSERT_FALSE (b1 <= b2);
    ASSERT_TRUE  (b1 >  b2);
    ASSERT_TRUE  (b1 >= b2);

    // b1 == b2
    b1.a.value = 1;
    b2.value3 = 1.5;
    b2.value3 -= 0.55;
    b2.value3 -= 0.95;

    ASSERT_TRUE  (b1 == b2);
    ASSERT_FALSE (b1 != b2);
    ASSERT_FALSE (b1 <  b2);
    ASSERT_TRUE  (b1 <= b2);
    ASSERT_FALSE (b1 >  b2);
    ASSERT_TRUE  (b1 >= b2);
}
