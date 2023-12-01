/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>

#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/SerializerJson.h>
#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QJsonValue>
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

using namespace SuitableStruct;

namespace {

struct CustomType
{
    int value{};

    Buffer ssSaveImpl() const {
        return Buffer::fromValue(value);
    }

    void ssLoadImpl(BufferReader& src) {
        src.read(value);
    }

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
    QJsonValue ssJsonSaveImpl() const {
        return value;
    }

    void ssJsonLoadImpl(const QJsonValue& src) {
        assert(src.isDouble());
        value = src.toInt();
    }
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY


    bool operator==(const CustomType& rhs) const { return value == rhs.value; }
    bool operator!=(const CustomType& rhs) const { return !(*this == rhs); }
};

struct Struct1
{
    CustomType a;
    CustomType b;

    auto ssTuple() const { return std::tie(a, b); }
    auto ssNamesTuple() const { return std::tie("a", "b"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct1);
};

} // namespace

TEST(SuitableStruct, CustomHandlers)
{
    Struct1 initial, deserialized;
    initial.b.value = 127;

    auto buffer = ssSave(initial);
    ASSERT_NE(initial, deserialized);
    ssLoad(buffer, deserialized);
    ASSERT_EQ(initial, deserialized);
}

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY

TEST(SuitableStruct, CustomHandlers_Json)
{
    Struct1 initial, deserialized;
    initial.b.value = 127;

    auto buffer = ssJsonSave(initial);
    ASSERT_NE(initial, deserialized);
    ssJsonLoad(buffer, deserialized);
    ASSERT_EQ(initial, deserialized);
}

#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
