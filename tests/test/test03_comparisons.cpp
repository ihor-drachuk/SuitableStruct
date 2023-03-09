#include <gtest/gtest.h>
#include <SuitableStruct/Comparisons.h>
#include <memory>
#include <optional>

namespace {

struct SomeStruct;
using SomeStructPtr = std::shared_ptr<SomeStruct>;

struct A
{
    int value {};
    auto ssTuple() const { return std::tie(value); }
    SS_COMPARISONS_MEMBER(A);
};

struct B
{
    A a;
    int value2 {};
    float value3 {};

    auto ssTuple() const { return std::tie(a, value2, value3); }
    SS_COMPARISONS_MEMBER(B);
};

struct AB : A, B
{
    int value4 {};
    std::shared_ptr<int> value5;

    auto ssLocalTuple() const { return std::tie(value4, value5); }
    auto ssTuple() const { return std::tuple_cat(
        static_cast<const A*>(this)->ssTuple(),
        static_cast<const B*>(this)->ssTuple(),
        ssLocalTuple());
    }
    SS_COMPARISONS_MEMBER(AB);
};

template<typename SmartType>
struct SP
{
    SmartType value;
    auto ssTuple() const { return std::tie(value); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(SP);
};

inline bool compare_eq(const SP<SomeStructPtr>&, const SomeStructPtr& a, const SomeStructPtr& b) { return a.get() == b.get(); }

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

    // AB
    AB ab1, ab2;
    ASSERT_TRUE  (b1 == b2);
    ASSERT_FALSE (b1 != b2);
}

TEST(SuitableStruct, ComparisonsTest_shared_ptr)
{
    SP<std::shared_ptr<int>> a, b;
    ASSERT_EQ(a, b);

    a.value = std::make_shared<int>(1);
    ASSERT_NE(a, b);

    b.value = std::make_shared<int>(2);
    ASSERT_NE(a, b);

    *a.value = 2;
    ASSERT_EQ(a, b);
}

TEST(SuitableStruct, ComparisonsTest_shared_ptr_specific)
{
    SP<SomeStructPtr> a, b;
    ASSERT_EQ(a, b);
}

TEST(SuitableStruct, ComparisonsTest_unique_ptr)
{
    SP<std::unique_ptr<int>> a, b;
    ASSERT_EQ(a, b);

    a.value = std::make_unique<int>(1);
    ASSERT_NE(a, b);

    b.value = std::make_unique<int>(2);
    ASSERT_NE(a, b);

    *a.value = 2;
    ASSERT_EQ(a, b);
}

TEST(SuitableStruct, ComparisonsTest_weak_ptr)
{
    std::shared_ptr<int> v1, v2;
    v1 = std::make_shared<int>(1);
    v2 = std::make_shared<int>(2);

    SP<std::weak_ptr<int>> a, b;
    ASSERT_EQ(a, b);

    a.value = v1;
    ASSERT_NE(a, b);

    b.value = v2;
    ASSERT_NE(a, b);

    *v1 = 2;
    ASSERT_EQ(a, b);
}

TEST(SuitableStruct, ComparisonsTest_optional)
{
    SP<std::optional<int>> a, b;
    ASSERT_EQ(a, b);

    a.value = 1;
    ASSERT_NE(a, b);

    b.value = 2;
    ASSERT_NE(a, b);

    a.value = 2;
    ASSERT_EQ(a, b);
}
