#include <gtest/gtest.h>
#include <SuitableStruct/Internals/LongSSO.h>

using namespace SuitableStruct;

TEST(SuitableStruct, LongSSO_tests)
{
    LongSSO a, b;
    LongSSO<2> c;
    ASSERT_EQ(a, b);
    ASSERT_EQ(a, c);

    a.appendData(reinterpret_cast<const uint8_t*>("a"), 1);
    c.appendData(reinterpret_cast<const uint8_t*>("a"), 1);
    ASSERT_NE(a, b);
    ASSERT_EQ(a, c);

    a.clear();
    ASSERT_EQ(a, b);
    ASSERT_NE(a, c);

    c.clear();
    ASSERT_EQ(a, c);

    a.appendData(reinterpret_cast<const uint8_t*>("Hello"), 5);
    c.appendData(reinterpret_cast<const uint8_t*>("Hello"), 5);
    ASSERT_NE(a, b);
    ASSERT_EQ(a, c);

    a.clear();
    ASSERT_EQ(a, b);
    ASSERT_NE(a, c);

    c.clear();
    ASSERT_EQ(a, c);
}
