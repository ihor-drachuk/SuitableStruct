/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

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
#include <QJsonDocument>
#include <QDateTime>
#include <QTimeZone>
#include <QColor>
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
    uint32_t c{};
    QString d;
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
    QJsonValue t1;
    QJsonObject t2;
    QJsonArray t3;
    QColor u;

    auto ssTuple() const { return std::tie(a0, a, b, c, d, e1, e2, e3, f, g, h, k, l, m, n, o, p, q, r, s1, s2, t1, t2, t3, u); }
    auto ssNamesTuple() const { return std::tie("a0", "a", "b", "c", "d", "e1", "e2", "e3", "f", "g", "h", "k", "l", "m", "n", "o", "p", "q", "r", "s1", "s2", "t1", "t2", "t3", "u"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct1);
};

struct Struct2
{
    int8_t a0{};
    int8_t a1{};

    uint8_t b0{};
    uint8_t b1{};

    int16_t c0{};
    int16_t c1{};

    uint16_t d0{};
    uint16_t d1{};

    int32_t e0{};
    int32_t e1{};

    uint32_t f0{};
    uint32_t f1{};

    int64_t g0{};
    int64_t g1{};

    uint64_t h0{};
    uint64_t h1{};

    auto ssTuple() const { return std::tie(a0, a1, b0, b1, c0, c1, d0, d1, e0, e1, f0, f1, g0, g1, h0, h1); }
    auto ssNamesTuple() const { return std::tie("a0", "a1", "b0", "b1", "c0", "c1", "d0", "d1", "e0", "e1", "f0", "f1", "g0", "g1", "h0", "h1"); }
};

const QJsonValue testJson = []() {
  auto configText = "{"
                    "\"content\": {"
                      "\"a0\": \"127\","
                      "\"a1\": \"-0x80\","

                      "\"b0\": \"255\","
                      "\"b1\": \"0x80\","

                      "\"c0\": \"12345\","
                      "\"c1\": \"-0x3039\","

                      "\"d0\": \"65535\","
                      "\"d1\": \"0xFFFF\","

                      "\"e0\": \"2147483647\","
                      "\"e1\": \"-0x75BCD15\","

                      "\"f0\": \"4294967295\","
                      "\"f1\": \"0x075BCD15\","

                      "\"g0\": \"-1234567890123456\","
                      "\"g1\": \"0x7FFFFFFFFFFFFFFF\","

                      "\"h0\": \"123456789012345678\","
                      "\"h1\": \"0xFFFFFFFFFFFFFFFF\""
                      "}"
                    "}";

  return QJsonDocument::fromJson(configText).object();
}();

} // namespace

TEST(SuitableStruct, JsonSerialization)
{
    const Struct1 empty;
    Struct1 a, b, c;
    ASSERT_EQ(a, b);

    a.a0 = true;
    a.a = 10;
    a.b = 15;
    a.c = 0x6E33168B;
    a.d = "Test";
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

    a.t1 = QJsonObject({{"SubValue1", 1}, {"SubValue2", "b"}});
    a.t2 = QJsonObject({{"SubValue3", 123}});
    a.t3 = QJsonArray{1, 2, QJsonObject({{"SubValue4", 816}})};
    a.u = QColor(128, 255, 0);

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

TEST(SuitableStruct, JsonDeserializationIntegrals)
{
    Struct2 a;
    ssJsonLoad(testJson, a, false);

    ASSERT_EQ(a.a0, 127);
    ASSERT_EQ(a.a1, -0x80);

    ASSERT_EQ(a.b0, 255);
    ASSERT_EQ(a.b1, 0x80);

    ASSERT_EQ(a.c0, 12345);
    ASSERT_EQ(a.c1, -0x3039);

    ASSERT_EQ(a.d0, 65535);
    ASSERT_EQ(a.d1, 0xFFFF);

    ASSERT_EQ(a.e0, 2147483647);
    ASSERT_EQ(a.e1, -0x75BCD15);

    ASSERT_EQ(a.f0, 4294967295);
    ASSERT_EQ(a.f1, 0x075BCD15);

    ASSERT_EQ(a.g0, -1234567890123456);
    ASSERT_EQ(a.g1, 0x7FFFFFFFFFFFFFFF);

    ASSERT_EQ(a.h0, 123456789012345678);
    ASSERT_EQ(a.h1, 0xFFFFFFFFFFFFFFFF);
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
