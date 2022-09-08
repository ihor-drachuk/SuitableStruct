#include <gtest/gtest.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <SuitableStruct/SerializerJson.h>
#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Containers/vector.h>
#include <SuitableStruct/Containers/list.h>
#include <SuitableStruct/Containers/array.h>
#include <QPoint>
#include <string>
#include <tuple>
#include <array>
#include <vector>
#include <list>
#include <optional>
#include <memory>

using namespace SuitableStruct;

//using Struct1 = int;

enum class Enum1 {
    Value1,
    Value2
};

namespace Test9_CustomEnum {
Q_NAMESPACE
enum class Enum2 {
    Value1,
    Value2,
    Value3
};
Q_ENUM_NS(Enum2);
} // namespace Test9_CustomEnum

namespace {

struct Struct1
{
    short a{};
    long b{};
    QString c;
    Enum1 e1;
    Test9_CustomEnum::Enum2 e2;
    Test9_CustomEnum::Enum2 e3;
    QByteArray f;
    double g{};
    QChar h;
    QPoint k;
    std::string l;
    std::vector<int> m;       // #include <SuitableStruct/Containers/vector.h>
    std::list<std::string> n; // #include <SuitableStruct/Containers/list.h>
    std::array<double, 5> o;  // #include <SuitableStruct/Containers/array.h>
    std::tuple<int, std::string> p;
    std::shared_ptr<int> q;
    std::unique_ptr<int> r;
    std::optional<int> s1, s2;

    auto ssTuple() const { return std::tie(a, b, c, e1, e2, e3, f, g, h, k, l, m, n, o, p, q, r, s1, s2); }
    auto ssNamesTuple() const { return std::tie("a", "b", "c", "e1", "e2", "e3", "f", "g", "h", "k", "l", "m", "n", "o", "p", "q", "r", "s1", "s2"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct1);
};

} // namespace

TEST(SuitableStruct, JsonSerialization)
{
    Struct1 a, b;
    ASSERT_EQ(a, b);

    a.a = 10;
    a.b = 15;
    a.c = "Test";
    a.e1 = Enum1::Value2;
    a.e2 = Test9_CustomEnum::Enum2::Value3;
    a.e3 = static_cast<Test9_CustomEnum::Enum2>(123123);
    a.f = QString("Test").toUtf8();
    a.g = 1.276;
    a.h = 'H';
    a.k = QPoint(1, 5);
    a.l = "Test2";
    a.m = {1,2,3};
    a.n = {"str1", "str2"};
    a.o = {.1, .2, .3, .4, .5};
    a.p = {11, "test"};
    a.q = std::make_shared<int>(12);
    a.r = std::make_unique<int>(13);
    a.s1 = 14;
    a.s2 = {};

    ASSERT_NE(a, b);
    auto saved = ssJsonSave(a);
    ssJsonLoad(saved, b);

    ASSERT_EQ(a, b);
    ASSERT_NE(a.q.get(), b.q.get());
}

#include "test09_json.moc"

#endif // #ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
