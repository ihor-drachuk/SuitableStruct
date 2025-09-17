/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>

#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Exceptions.h>
#include <cmath>

using namespace SuitableStruct;

namespace {

struct CustomStruct_v0
{
    int value {};

    // Custom serialization methods - NO ssTuple
    Buffer ssSaveImpl() const {
        return Buffer::fromValue(value);
    }

    void ssLoadImpl(BufferReader& src) {
        src.read(value);
    }

    bool operator==(const CustomStruct_v0& rhs) const { return value == rhs.value; }
    bool operator!=(const CustomStruct_v0& rhs) const { return !(*this == rhs); }
};

struct CustomStruct_v1
{
    int value {};
    float extra {};

    using ssVersions = std::tuple<CustomStruct_v0, CustomStruct_v1>;

    // Custom serialization methods - NO ssTuple
    Buffer ssSaveImpl() const {
        Buffer result;
        result += Buffer::fromValue(value);
        result += Buffer::fromValue(extra);
        return result;
    }

    void ssLoadImpl(BufferReader& src) {
        src.read(value);
        src.read(extra);
    }

    // Version conversion methods
    void ssUpgradeFrom(const CustomStruct_v0& prev) {
        value = prev.value;
        extra = 1.3f;
    }

    void ssDowngradeTo(CustomStruct_v0& next) const {
        next.value = value;
    }

    bool operator==(const CustomStruct_v1& rhs) const {
        return value == rhs.value && std::abs(extra - rhs.extra) < 1e-6f;
    }
    bool operator!=(const CustomStruct_v1& rhs) const { return !(*this == rhs); }
};

struct CustomStruct_v2
{
    int value {};
    double precision_extra {};
    std::string description;

    using ssVersions = std::tuple<CustomStruct_v0, CustomStruct_v1, CustomStruct_v2>;

    // Custom serialization methods - NO ssTuple
    Buffer ssSaveImpl() const {
        Buffer result;
        result += Buffer::fromValue(value);
        result += Buffer::fromValue(precision_extra);
        result += ssSave(description, false);
        return result;
    }

    void ssLoadImpl(BufferReader& src) {
        src.read(value);
        src.read(precision_extra);
        ssLoad(src, description, SSLoadMode::NonProtectedDefault);
    }

    // Version conversion methods
    void ssUpgradeFrom(const CustomStruct_v1& prev) {
        value = prev.value;
        precision_extra = static_cast<double>(prev.extra);
        description = "upgraded";
    }

    void ssDowngradeTo(CustomStruct_v1& next) const {
        next.value = value;
        next.extra = static_cast<float>(precision_extra);
    }

    bool operator==(const CustomStruct_v2& rhs) const {
        return value == rhs.value && std::abs(precision_extra - rhs.precision_extra) < 1e-9 && description == rhs.description;
    }
    bool operator!=(const CustomStruct_v2& rhs) const { return !(*this == rhs); }
};


} // namespace

TEST(SuitableStruct, CustomHandlers_Versioning_v0_to_v1)
{
    CustomStruct_v0 v0;
    v0.value = 42;

    const auto buffer = ssSave(v0);

    CustomStruct_v1 v1;
    ssLoad(buffer, v1);

    ASSERT_EQ(v1.value, 42);
    ASSERT_FLOAT_EQ(v1.extra, 1.3f);
}

TEST(SuitableStruct, CustomHandlers_Versioning_v1_to_v2)
{
    CustomStruct_v1 v1;
    v1.value = 100;
    v1.extra = 3.14f;

    const auto buffer = ssSave(v1);

    CustomStruct_v2 v2;
    ssLoad(buffer, v2);

    ASSERT_EQ(v2.value, 100);
    ASSERT_FLOAT_EQ(v2.precision_extra, 3.14);
    ASSERT_EQ(v2.description, "upgraded");
}

TEST(SuitableStruct, CustomHandlers_Versioning_v0_to_v2)
{
    CustomStruct_v0 v0;
    v0.value = 77;

    const auto buffer = ssSave(v0);

    CustomStruct_v2 v2;
    ssLoad(buffer, v2);

    ASSERT_EQ(v2.value, 77);
    ASSERT_FLOAT_EQ(v2.precision_extra, 1.3);
    ASSERT_EQ(v2.description, "upgraded");
}

TEST(SuitableStruct, CustomHandlers_Versioning_v2_to_v1)
{
    CustomStruct_v2 v2;
    v2.value = 200;
    v2.precision_extra = 2.71828;
    v2.description = "test";

    const auto buffer = ssSave(v2);

    CustomStruct_v1 v1;
    ssLoad(buffer, v1);

    ASSERT_EQ(v1.value, 200);
    ASSERT_FLOAT_EQ(v1.extra, 2.71828f);
}

TEST(SuitableStruct, CustomHandlers_Versioning_v2_to_v0)
{
    CustomStruct_v2 v2;
    v2.value = 150;
    v2.precision_extra = 1.41421;
    v2.description = "pi";

    const auto buffer = ssSave(v2);

    CustomStruct_v0 v0;
    ssLoad(buffer, v0);

    ASSERT_EQ(v0.value, 150);
}

TEST(SuitableStruct, CustomHandlers_Versioning_v1_to_v0)
{
    CustomStruct_v1 v1;
    v1.value = 88;
    v1.extra = 9.99f;

    const auto buffer = ssSave(v1);

    CustomStruct_v0 v0;
    ssLoad(buffer, v0);

    ASSERT_EQ(v0.value, 88);
}

TEST(SuitableStruct, CustomHandlers_Versioning_SameVersion_v1)
{
    CustomStruct_v1 original;
    original.value = 55;
    original.extra = 7.77f;

    const auto buffer = ssSave(original);

    CustomStruct_v1 restored;
    ssLoad(buffer, restored);

    ASSERT_EQ(original, restored);
}

TEST(SuitableStruct, CustomHandlers_Versioning_SameVersion_v2)
{
    CustomStruct_v2 original;
    original.value = 333;
    original.precision_extra = 12.345;
    original.description = "same version test";

    const auto buffer = ssSave(original);

    CustomStruct_v2 restored;
    ssLoad(buffer, restored);

    ASSERT_EQ(original, restored);
}

