#include <gtest/gtest.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <SuitableStruct/SerializerJson.h>
#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Containers/vector.h>
#include <SuitableStruct/Containers/list.h>
#include <SuitableStruct/Containers/array.h>
#include <QPoint>
#include <QJsonValue>
#include <QJsonObject>
#include <QDateTime>
#include <QTimeZone>
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
    bool a0{};
    short a{};
    long b{};
    QString c;
    QJsonValue d;
    Enum1 e1{};
    Test9_CustomEnum::Enum2 e2{};
    Test9_CustomEnum::Enum2 e3{};
    QByteArray f;
    double g{};
    QChar h;
    // missing i, j
    QPoint k;
    std::string l;
    std::vector<int> m;         // #include <SuitableStruct/Containers/vector.h>
    std::list<std::string> n;   // #include <SuitableStruct/Containers/list.h>
    std::array<double, 5> o{};  // #include <SuitableStruct/Containers/array.h>
    std::tuple<int, std::string> p {};
    std::shared_ptr<int> q;
    std::unique_ptr<int> r;
    std::optional<int> s1, s2;

    auto ssTuple() const { return std::tie(a0, a, b, c, d, e1, e2, e3, f, g, h, k, l, m, n, o, p, q, r, s1, s2); }
    auto ssNamesTuple() const { return std::tie("a0", "a", "b", "c", "d", "e1", "e2", "e3", "f", "g", "h", "k", "l", "m", "n", "o", "p", "q", "r", "s1", "s2"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct1);
};

} // namespace

TEST(SuitableStruct, JsonSerialization)
{
    const Struct1 empty;
    Struct1 a, b, c;
    ASSERT_EQ(a, b);

    a.a0 = true;
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
    a.d = QJsonObject({{"SubValue1", 1}, {"SubValue2", "b"}});

    ASSERT_NE(b, a);
    auto saved = ssJsonSave(a);
    ssJsonLoad(saved, b);

    ASSERT_EQ(b, a);
    ASSERT_NE(b.q.get(), a.q.get());

    { // Integrity tests
        auto valueRoot = saved.toObject(); // content, hash
        auto valueContent1 = valueRoot["content"].toObject(); // content, version
        auto valueContent2 = valueContent1["content"].toObject(); // a, b, c, ...
        valueContent2["a"] = a.a + 1;
        valueContent1["content"] = valueContent2;
        valueRoot["content"] = valueContent1;
        auto saved2 = QJsonValue(valueRoot);
        ASSERT_THROW(ssJsonLoad(saved2, c), std::runtime_error);
        ASSERT_EQ(c, empty);

        valueContent2["a"] = a.a;
        valueContent1["content"] = valueContent2;
        valueRoot["content"] = valueContent1;
        saved2 = QJsonValue(valueRoot);
        ssJsonLoad(saved2, c);
        ASSERT_EQ(c, a);
    }
}

TEST(SuitableStruct, JsonSerialization_DateTime)
{
    const QDate date(2023, 05, 05);
    const QTime time(1, 54, 12, 127);

    const std::vector<QDateTime> testData {
        QDateTime::currentDateTime(),
        QDateTime::currentDateTimeUtc(),
        QDateTime(date, time, Qt::LocalTime),
        QDateTime(date, time, Qt::UTC),
        QDateTime(date, time, Qt::OffsetFromUTC, 60*60*2),
        QDateTime(date, time, QTimeZone("America/New_York")),
        QDateTime(date, time, QTimeZone("sdfgihdsfg"))
    };

    for (const auto& x : testData) {
        const auto buffer = ssJsonSave(x, false);
        const auto readBack = ssJsonLoadRet<QDateTime>(buffer, false);
        ASSERT_EQ(x, readBack);
        ASSERT_EQ(x.offsetFromUtc(), readBack.offsetFromUtc());
        ASSERT_EQ(x.timeZone(), readBack.timeZone());
    }
}

TEST(SuitableStruct, JsonSerialization_OldBoolCompatibility)
{
    const QJsonValue oldTrue = 1;
    const QJsonValue oldFalse = 0;
    const auto readTrue = ssJsonLoadImplRet<bool>(oldTrue);
    const auto readFalse = ssJsonLoadImplRet<bool>(oldFalse);
    ASSERT_TRUE(readTrue);
    ASSERT_FALSE(readFalse);
}

#include "test09_json.moc"

#endif // #ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
