/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>

#include <SuitableStruct/Containers/array.h>
#include <SuitableStruct/Containers/deque.h>
#include <SuitableStruct/Containers/forward_list.h>
#include <SuitableStruct/Containers/list.h>
#include <SuitableStruct/Containers/vector.h>
#include <SuitableStruct/Containers/set.h>
#include <SuitableStruct/Containers/unordered_set.h>
#include <SuitableStruct/Containers/multiset.h>
#include <SuitableStruct/Containers/unordered_multiset.h>
#include <SuitableStruct/Containers/unordered_map.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QList>
#include <QVector>
#include <QStringList>
#include <SuitableStruct/Containers/QSet.h>
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

using namespace SuitableStruct;

struct Struct1
{
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    QList<int> a;
    QVector<int> b;
    QSet<int> c;
    QStringList d;
#else
    int a {};
    int b {};
    int c {};
    int d {};
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

    auto ssTuple() const { return std::tie(a, b, c, d); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct1)
};

struct Struct2
{
    std::vector<Struct1> first;

    std::array<int, 3> a {};
    std::deque<int> b;
    std::forward_list<int> c;
    std::list<int> d;
    std::vector<int> e;
    std::set<int> f;
    std::unordered_set<int> g;
    std::multiset<int> h;
    std::unordered_multiset<int> i;
    std::unordered_map<int, std::string> j;

    auto ssTuple() const { return std::tie(first, a, b, c, d, e, f, g, h, i, j); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct2)
};

struct Struct3
{
    std::tuple<int, float, Struct2> value;

    auto ssTuple() const { return std::tie(value); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct3)
};

TEST(SuitableStruct, ContainersTest)
{
    Struct2 a1, a2;

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    a1.first = {{{1},{2},{3},{"4"}}};
#else
    a1.first = {{1,2,3,4}};
#endif

    a1.b = {2, 22};
    a1.c = {3, 33};
    a1.d = {4, 44};
    a1.e = {5, 55};
    a1.f = {6, 66};
    a1.g = {7, 77};
    a1.h = {8, 88};
    a1.i = {9, 99};
    a1.j = {{10, "10"}, {11, "11"}};

    auto buf = ssSave(a1);
    ssLoad(buf, a2);

    ASSERT_EQ(a1, a2);
}

TEST(SuitableStruct, TupleTest)
{
    Struct3 value1, value2;
    auto& [a, b, c] = value1.value;
    a = 1;
    b = 2.34f;
    c.d = {1,2,3};

    auto buf = ssSave(value1);
    ssLoad(buf, value2);
    ASSERT_EQ(value1, value2);
}
