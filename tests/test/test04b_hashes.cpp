/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <string>
#include <unordered_set>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Hashes.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QJsonObject>
#include <SuitableStruct/SerializerJson.h>
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

using namespace SuitableStruct;

namespace {

struct SomeStruct1
{
    int a {};
    std::string b;

    auto ssTuple() const { return std::tie(a, b); }
    auto ssNamesTuple() const { return std::tie("a", "b"); }

    // For test
    bool operator==(const SomeStruct1& rhs) const { return (ssTuple() == rhs.ssTuple()); }
};

struct SomeStruct2
{
    SomeStruct1 struct1;
    float c {};
    std::shared_ptr<int> d;

    auto ssTuple() const { return std::tie(struct1, c, d); }
    auto ssNamesTuple() const { return std::tie("struct1", "c", "d"); }

    // For test
    bool operator==(const SomeStruct2& rhs) const { return (ssTuple() == rhs.ssTuple()); }
};

} // namespace

SS_HASHES(SomeStruct2); // Enable std::hash for it.
                        // E.g. enable use in std::unordered_set.


TEST(SuitableStruct, Hashes_Binary)
{
    // Random data
    const uint8_t data[] = { 0x93, 0xF0, 0x00, 0x01, 0x00, 0x92, 0xE1, 0x2F };
    constexpr size_t dataSize = sizeof(data);

    const uint32_t hashF0 = ssHashRaw_F0(data, dataSize);
    const uint32_t hashF1 = ssHashRaw(data, dataSize);
    ASSERT_NE(hashF0, hashF1);

    EXPECT_EQ(hashF0, 0x5B39D300);
    EXPECT_EQ(hashF1, 0xA8824A83);
}

TEST(SuitableStruct, Hashes_Structures)
{
    SomeStruct2 s2;
    s2.struct1.a = 1;
    s2.struct1.b = "Hello";
    s2.c = 3.14f;
    s2.d = std::make_shared<int>(15);

    const auto h1 = ssHash(s2);
    *s2.d = 20;
    const auto h2 = ssHash(s2);

    ASSERT_NE(h1, h2);
    ASSERT_EQ(h2, ssHash(s2));
    //EXPECT_EQ(h2, 0x4BF822A0); // Value for x86 MSVC.

    s2.struct1.a = 2;
    const auto h3 = ssHash(s2);
    ASSERT_NE(h3, h1);

    // Make sure hash is STL-compatible
    std::unordered_set<SomeStruct2> set;
    set.insert(s2);
}

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY

TEST(SuitableStruct, Hashes_Json)
{
    QJsonArray array;
    array.append(1);
    array.append("Hello");
    array.append(3.14);
    array.append(true);
    array.append({});
    array.append(QJsonObject{{"key", "value"}});

    QJsonObject nestedObj;
    nestedObj["nestedValue"] = 1;
    nestedObj["array"] = array;

    QJsonObject obj;
    obj["nested"] = nestedObj;
    obj["b"] = 2;
    obj["c"] = 3.14159;

    const auto hashF0 = Internal::ssJsonHashValue_F0(obj);
    const auto hashF1 = Internal::ssJsonHashValue(obj);
    ASSERT_NE(hashF0, hashF1);

    EXPECT_EQ(hashF0, 0x413BDA13);
    EXPECT_EQ(hashF1, 0x0E3B77B4);

    // Test with nested object
    nestedObj["nestedValue"] = 2;
    obj["nested"] = nestedObj;

    const auto hash2 = Internal::ssJsonHashValue(obj);
    ASSERT_NE(hash2, hashF1);
}

#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
