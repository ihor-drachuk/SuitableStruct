/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>
#include <vector>
#include <optional>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QJsonValue>
#include <QJsonObject>
#include <QDateTime>
#include <QTimeZone>
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

using namespace SuitableStruct;

namespace {

struct SomeStruct1
{
    int a {};
    std::string b;

    auto ssTuple() const { return std::tie(a, b); }
};

SS_COMPARISONS(SomeStruct1);

struct SomeStruct2
{
    SomeStruct1 struct1;
    float c {};
    std::optional<int> d;
    bool e {};
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    QJsonValue f;
#else
    int f {};
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

    auto ssTuple() const { return std::tie(struct1, c, d, e, f); }
};

SS_COMPARISONS_ONLY_EQ(SomeStruct2);

} // namespace


TEST(SuitableStruct, SerializationTest)
{
    SomeStruct2 value1;

    value1.struct1.a = 1;
    value1.struct1.b = "sdfsdf";
    value1.c = 2.5;
    value1.d = 150;
    value1.e = true;
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    value1.f = QJsonObject({{"SubValue1", 1}, {"SubValue2", "b"}});
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

    SomeStruct2 value2;
    ASSERT_NE(value1, value2);

    auto saved = ssSave(value1);
    BufferReader reader(saved);
    ssLoad(reader, value2);
    ASSERT_EQ(reader.rest(), 0);

    ASSERT_EQ(value1, value2);

    value1.d.reset();

    saved = ssSave(value1);
    reader.resetPosition();
    ssLoad(reader, value2);

    ASSERT_EQ(value1, value2);
}

TEST(SuitableStruct, SerializationTest_Corruption)
{
    auto trySaveLoad = [](int byteToCorrupt = -1, int reduceSize = 0) {
        SomeStruct2 value1;

        value1.struct1.a = 1;
        value1.struct1.b = "sdfsdf";
        value1.c = 2.5;

        SomeStruct2 value2;
        ASSERT_NE(value1, value2);

        auto saved = ssSave(value1);

        if (byteToCorrupt >= 0) {
            assert((unsigned)byteToCorrupt < saved.size());
            saved.data()[byteToCorrupt]++;
        }

        saved.reduceSize(reduceSize);

        ASSERT_THROW(ssLoad(saved, value2), std::exception);
    };

    for (int i = 0; i < 20; i++) {
        trySaveLoad(i);
    }

    for (int i = 1; i < 20; i++) {
        trySaveLoad(-1, i);
    }
}

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
TEST(SuitableStruct, SerializationTest_DateTime)
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
        const auto buffer = ssSave(x, false);
        const auto readBack = ssLoadRet<QDateTime>(buffer, false);
        ASSERT_EQ(x, readBack);
        ASSERT_EQ(x.offsetFromUtc(), readBack.offsetFromUtc());
        ASSERT_EQ(x.timeZone(), readBack.timeZone());
    }
}
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

TEST(SuitableStruct, SerializationTest_OldBoolCompatibility)
{
    const Buffer oldTrue("\1", 1);
    const Buffer oldFalse("\0", 1);
    BufferReader oldTrueReader(oldTrue);
    BufferReader oldFalseReader(oldFalse);
    const auto readTrue = ssLoadImplRet<bool>(BufferReader(oldTrue));
    const auto readFalse = ssLoadImplRet<bool>(BufferReader(oldFalse));
    ASSERT_TRUE(readTrue);
    ASSERT_FALSE(readFalse);
}
