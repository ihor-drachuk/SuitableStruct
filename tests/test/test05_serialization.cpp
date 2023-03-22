#include <gtest/gtest.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>
#include <optional>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QJsonValue>
#include <QJsonObject>
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
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    QJsonValue e;
#else
    int e {};
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

    auto ssTuple() const { return std::tie(struct1, c, d, e); }
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
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    value1.e = QJsonObject({{"SubValue1", 1}, {"SubValue2", "b"}});
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
