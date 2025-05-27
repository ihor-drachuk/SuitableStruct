/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>

using namespace SuitableStruct;

namespace {

template<typename T>
static bool compareFloat(T a, T b) {
    return std::abs(a - b) <= std::numeric_limits<T>::epsilon();
}

// Plain structs without any serialization methods
struct Struct_v0
{
    int a{};
    float b{};

    bool operator==(const Struct_v0& rhs) const {
        return a == rhs.a && compareFloat(b, rhs.b);
    }
};

struct Struct_v1
{
    int a{};
    float b{};
    std::string c;

    bool operator==(const Struct_v1& rhs) const {
        return a == rhs.a && compareFloat(b, rhs.b) && c == rhs.c;
    }
};

struct Struct_v2
{
    int a{};
    float b{};
    std::string c;
    double d{};

    bool operator==(const Struct_v2& rhs) const {
        return a == rhs.a &&
               compareFloat(b, rhs.b) &&
               c == rhs.c &&
               compareFloat(d, rhs.d);
    }
};

} // namespace

// External handlers for version 0
namespace SuitableStruct {
template<>
struct Handlers<Struct_v0> : public std::true_type
{
    static Buffer ssSaveImpl(const Struct_v0& v) {
        Buffer buf;
        buf.write(v.a);
        buf.write(v.b);
        return buf;
    }

    static void ssLoadImpl(BufferReader& src, Struct_v0& v) {
        src.read(v.a);
        src.read(v.b);
    }
};
} // namespace SuitableStruct

// External handlers for version 1
namespace SuitableStruct {
template<>
struct Handlers<Struct_v1> : public std::true_type
{
    using ssVersions = std::tuple<Struct_v0, Struct_v1>;

    static Buffer ssSaveImpl(const Struct_v1& v) {
        Buffer buf;
        buf.write(v.a);
        buf.write(v.b);
        buf.write(static_cast<uint64_t>(v.c.size()));
        buf.writeRaw(v.c.data(), v.c.size());
        return buf;
    }

    static void ssLoadImpl(BufferReader& src, Struct_v1& v) {
        src.read(v.a);
        src.read(v.b);
        uint64_t sz;
        src.read(sz);
        v.c.resize(sz);
        src.readRaw(v.c.data(), sz);
    }

    // Conversion methods in handlers
    static void ssUpgradeFrom(const Struct_v0& prev, Struct_v1& current) {
        current.a = prev.a;
        current.b = prev.b;
        current.c = "default";
    }

    static void ssDowngradeTo(const Struct_v1& current, Struct_v0& downgraded) {
        downgraded.a = current.a;
        downgraded.b = current.b;
    }
};
} // namespace SuitableStruct

// External handlers for version 2
namespace SuitableStruct {
template<>
struct Handlers<Struct_v2> : public std::true_type
{
    using ssVersions = std::tuple<Struct_v0, Struct_v1, Struct_v2>;

    static Buffer ssSaveImpl(const Struct_v2& v) {
        Buffer buf;
        buf.write(v.a);
        buf.write(v.b);
        buf.write(static_cast<uint64_t>(v.c.size()));
        buf.writeRaw(v.c.data(), v.c.size());
        buf.write(v.d);
        return buf;
    }

    static void ssLoadImpl(BufferReader& src, Struct_v2& v) {
        src.read(v.a);
        src.read(v.b);
        uint64_t sz;
        src.read(sz);
        v.c.resize(sz);
        src.readRaw(v.c.data(), sz);
        src.read(v.d);
    }

    // Conversion methods in handlers
    static void ssUpgradeFrom(const Struct_v1& prev, Struct_v2& current) {
        current.a = prev.a;
        current.b = prev.b;
        current.c = prev.c;
        current.d = 0.0;
    }

    static void ssDowngradeTo(const Struct_v2& current, Struct_v1& downgraded) {
        downgraded.a = current.a;
        downgraded.b = current.b;
        downgraded.c = current.c;
    }
};
} // namespace SuitableStruct

TEST(SuitableStruct, CustomHandlersVersioning_SameVersion)
{
    // Test saving and loading v0
    {
        Struct_v0 initial, deserialized;
        initial.a = 42;
        initial.b = 3.14f;

        const auto buffer = ssSave(initial);
        ssLoad(buffer, deserialized);
        ASSERT_EQ(initial, deserialized);
    }

    // Test saving and loading v1
    {
        Struct_v1 initial, deserialized;
        initial.a = 42;
        initial.b = 3.14f;
        initial.c = "test";

        const auto buffer = ssSave(initial);
        ssLoad(buffer, deserialized);
        ASSERT_EQ(initial, deserialized);
    }

    // Test saving and loading v2
    {
        Struct_v2 initial, deserialized;
        initial.a = 42;
        initial.b = 3.14f;
        initial.c = "test";
        initial.d = 2.718;

        const auto buffer = ssSave(initial);
        ssLoad(buffer, deserialized);
        ASSERT_EQ(initial, deserialized);
    }
}

TEST(SuitableStruct, CustomHandlersVersioning_MiddleVersion)
{
    // Test saving middle version (v1) and loading into new version
    {
        Struct_v1 middle;
        middle.a = 42;
        middle.b = 3.14f;
        middle.c = "test";

        const auto buffer = ssSave(middle);

        Struct_v2 deserialized;
        ssLoad(buffer, deserialized);

        ASSERT_EQ(deserialized.a, middle.a);
        ASSERT_FLOAT_EQ(deserialized.b, middle.b);
        ASSERT_EQ(deserialized.c, middle.c);
        ASSERT_DOUBLE_EQ(deserialized.d, 0.0);
    }

    // Test saving middle version (v1) and loading into old version
    {
        Struct_v1 middle;
        middle.a = 42;
        middle.b = 3.14f;
        middle.c = "test";

        const auto buffer = ssSave(middle);

        Struct_v0 old;
        ssLoad(buffer, old);

        ASSERT_EQ(old.a, middle.a);
        ASSERT_FLOAT_EQ(old.b, middle.b);
    }
}

TEST(SuitableStruct, CustomHandlersVersioning_MultipleUpgrades)
{
    // Test multiple version upgrades in sequence
    Struct_v0 v0;
    v0.a = 42;
    v0.b = 3.14f;

    const auto buffer = ssSave(v0);

    // First upgrade to v1
    Struct_v1 v1;
    ssLoad(buffer, v1);
    ASSERT_EQ(v1.a, v0.a);
    ASSERT_FLOAT_EQ(v1.b, v0.b);
    ASSERT_EQ(v1.c, "default");

    // Then upgrade to v2
    Struct_v2 v2;
    ssLoad(buffer, v2);
    ASSERT_EQ(v2.a, v0.a);
    ASSERT_FLOAT_EQ(v2.b, v0.b);
    ASSERT_EQ(v2.c, "default");
    ASSERT_DOUBLE_EQ(v2.d, 0.0);
}

TEST(SuitableStruct, CustomHandlersVersioning_MultipleDowngrades)
{
    // Test multiple version downgrades in sequence
    Struct_v2 v2;
    v2.a = 42;
    v2.b = 3.14f;
    v2.c = "test";
    v2.d = 2.718;

    const auto buffer = ssSave(v2);

    // First downgrade to v1
    Struct_v1 v1;
    ssLoad(buffer, v1);
    ASSERT_EQ(v1.a, v2.a);
    ASSERT_FLOAT_EQ(v1.b, v2.b);
    ASSERT_EQ(v1.c, v2.c);

    // Then downgrade to v0
    Struct_v0 v0;
    ssLoad(buffer, v0);
    ASSERT_EQ(v0.a, v2.a);
    ASSERT_FLOAT_EQ(v0.b, v2.b);
}
