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
#include <SuitableStruct/Containers/unordered_map.h>
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
#include <chrono>
#include <iostream>

using namespace std::chrono_literals;
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
    std::tuple<int, std::string> p;
    std::shared_ptr<int> q;
    std::unique_ptr<int> r;
    std::optional<int> s1, s2;
    QJsonValue t1;
    QJsonObject t2;
    QJsonArray t3;
    QColor u;
    std::unordered_map<int, std::string> v; // #include <SuitableStruct/Containers/unordered_map.h>
    std::chrono::steady_clock::time_point w1;
    std::chrono::system_clock::time_point w2;
    std::chrono::hours w3 {};
    std::chrono::milliseconds w4 {};
    std::variant<std::monostate, int, std::string> x;

    auto ssTuple() const { return std::tie(a0, a, b, c, d, e1, e2, e3, f, g, h, k, l, m, n, o, p, q, r, s1, s2, t1, t2, t3, u, v, w1, w2, w3, w4); }
    auto ssNamesTuple() const { return std::tie("a0", "a", "b", "c", "d", "e1", "e2", "e3", "f", "g", "h", "k", "l", "m", "n", "o", "p", "q", "r", "s1", "s2", "t1", "t2", "t3", "u", "v", "w1", "w2", "w3", "w4"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct1)
};

inline bool compare_eq(const Struct1&, const std::chrono::steady_clock::time_point& a, const std::chrono::steady_clock::time_point& b)
{
    const auto diff = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(a - b).count());
    return diff <= 1;
}

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
    int32_t e2{};

    uint32_t f0{};
    uint32_t f1{};

    int64_t g0{};
    int64_t g1{};
    int64_t g2{};

    uint64_t h0{};
    uint64_t h1{};

    auto ssTuple() const { return std::tie(a0, a1, b0, b1, c0, c1, d0, d1, e0, e1, e2, f0, f1, g0, g1, g2, h0, h1); }
    auto ssNamesTuple() const { return std::tie("a0", "a1", "b0", "b1", "c0", "c1", "d0", "d1", "e0", "e1", "e2", "f0", "f1", "g0", "g1", "g2", "h0", "h1"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct2)
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

    a.v = {{1, "one"}, {2, "two"}};

    a.w1 = std::chrono::steady_clock::now();
    a.w2 = std::chrono::system_clock::now() + 10min;
    a.w3 = std::chrono::hours(5);
    a.w4 = std::chrono::milliseconds(123);

    a.x = "asdasd";

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

TEST(SuitableStruct, JsonSerialization_Integers)
{
    const QJsonValue testJson = []() {
        const auto configText = R"(
        {
            "content": {
                "a0": "127",
                "a1": "-0x80",

                "b0": "255",
                "b1": "0x80",

                "c0": "12345",
                "c1": "-0x3039",

                "d0": "65535",
                "d1": "0xFFFF",

                "e0": "2147483647",
                "e1": "-0x75BCD15",
                "e2": "-2147483648",

                "f0": "4294967295",
                "f1": "0x075BCD15",

                "g0": "-1234567890123456",
                "g1": "0x7FFFFFFFFFFFFFFF",
                "g2": "-9223372036854775808",

                "h0": "123456789012345678",
                "h1": "0xFFFFFFFFFFFFFFFF"
            }
        }
      )";

      return QJsonDocument::fromJson(configText).object();
    }();

    Struct2 a, b;
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
    ASSERT_EQ(a.e2, -2147483648);

    ASSERT_EQ(a.f0, 4294967295);
    ASSERT_EQ(a.f1, 0x075BCD15);

    ASSERT_EQ(a.g0, -1234567890123456);
    ASSERT_EQ(a.g1, 0x7FFFFFFFFFFFFFFF);
    ASSERT_EQ(a.g2, /*-9223372036854775808LL*/ INT64_MIN); // Can't be specified normally

    ASSERT_EQ(a.h0, 123456789012345678);
    ASSERT_EQ(a.h1, 0xFFFFFFFFFFFFFFFF);

    const auto serialized = QString::fromLatin1(QJsonDocument(ssJsonSave(a, false).toObject()).toJson(QJsonDocument::JsonFormat::Indented));
    const auto reference = R"({
    "content": {
        "a0": 127,
        "a1": -128,
        "b0": 255,
        "b1": 128,
        "c0": 12345,
        "c1": -12345,
        "d0": 65535,
        "d1": 65535,
        "e0": 2147483647,
        "e1": -123456789,
        "e2": -2147483648,
        "f0": "4294967295",
        "f1": "123456789",
        "g0": "-1234567890123456",
        "g1": "9223372036854775807",
        "g2": "-9223372036854775808",
        "h0": "123456789012345678",
        "h1": "18446744073709551615"
    },
    "version": 0
}
)";
    ASSERT_EQ(serialized, reference);

    const auto testJson2 = QJsonDocument::fromJson(reference).object();
    ssJsonLoad(testJson2, b, false);
    ASSERT_EQ(a, b);
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


//#define GENERATE_MODE
TEST(SuitableStruct, JsonSerialization_PreSerializedSteadyClock)
{
    // Fixed reference time
    constexpr auto referenceEpochMs = 1704110400000LL; // 2024-01-01 12:00:00 UTC in milliseconds
    const auto referenceSystemTime = std::chrono::system_clock::time_point(std::chrono::milliseconds(referenceEpochMs));

#ifdef GENERATE_MODE
    // Generation mode - create test data for the reference time
    const auto currentSystemTime = std::chrono::system_clock::now();
    const auto currentSteadyTime = std::chrono::steady_clock::now();

    // Calculate what steady_clock time would represent our reference time
    const auto diffToReference = referenceSystemTime - currentSystemTime;
    const auto referenceSteadyTime = currentSteadyTime + diffToReference;

    const auto serialized = ssJsonSave(referenceSteadyTime, false);
    const auto jsonDoc = QJsonDocument(serialized.toObject());
    const auto jsonString = jsonDoc.toJson(QJsonDocument::Compact);

    std::cout << "\n=== Pre-serialized steady_clock JSON ===\n";
    std::cout << "Reference epoch ms: " << referenceEpochMs << "\n";
    std::cout << "JSON: " << jsonString.toStdString() << "\n";
    std::cout << "C++ literal: R\"JSON(" << jsonString.toStdString() << ")JSON\"\n";
    std::cout << "=====================================\n" << std::endl;

    // In generation mode, verify round-trip works
    const auto loadedTime = ssJsonLoadRet<std::chrono::steady_clock::time_point>(serialized, false);
    const auto loadedDiff = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(loadedTime - referenceSteadyTime).count());
    ASSERT_LE(loadedDiff, 1);  // Should be nearly identical
#else
    const char* preSerializedJson = R"JSON({"content":{"Timepoint_version_marker":"0xFFFFFFFFFFFFFFFF52810BD50C38E940","data":{"content":"1704110400000000200","version":0}},"version":0})JSON";
    const auto jsonDoc = QJsonDocument::fromJson(preSerializedJson);
    const auto jsonValue = QJsonValue(jsonDoc.object());

    const auto loadedTime = ssJsonLoadRet<std::chrono::steady_clock::time_point>(jsonValue, false);

    // Convert loaded steady_clock time back to system_clock to verify
    const auto currentSystemTime = std::chrono::system_clock::now();
    const auto currentSteadyTime = std::chrono::steady_clock::now();
    const auto loadedSystemTime = currentSystemTime + (loadedTime - currentSteadyTime);

    // The loaded time should represent our reference time
    const auto diffMs = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(loadedSystemTime - referenceSystemTime).count());
    ASSERT_LE(diffMs, 1000);  // Allow 1 second tolerance for clock drift and test execution time
#endif // GENERATE_MODE
}

#include "test09_json.moc"

#endif // #ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
