/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>

using namespace SuitableStruct;

namespace {

// Inner struct versioning
struct InnerStruct_v0
{
    int value {};

    using ssVersions = std::tuple<InnerStruct_v0>;
    auto ssTuple() const { return std::tie(value); }
    SS_COMPARISONS_MEMBER(InnerStruct_v0)
};

struct InnerStruct_v1
{
    int value {};
    float extra {};

    using ssVersions = std::tuple<InnerStruct_v0, InnerStruct_v1>;
    auto ssTuple() const { return std::tie(value, extra); }

    void ssUpgradeFrom(const InnerStruct_v0& prev) {
        value = prev.value;
        extra = 1.5f;
    }

    void ssDowngradeTo(InnerStruct_v0& next) const {
        next.value = value;
    }

    SS_COMPARISONS_MEMBER(InnerStruct_v1)
};

struct InnerStruct_v2
{
    int value {};
    double extra {};
    std::string tag;

    using ssVersions = std::tuple<InnerStruct_v0, InnerStruct_v1, InnerStruct_v2>;
    auto ssTuple() const { return std::tie(value, extra, tag); }

    void ssUpgradeFrom(const InnerStruct_v1& prev) {
        value = prev.value;
        extra = static_cast<double>(prev.extra);
        tag = "upgraded";
    }

    void ssDowngradeTo(InnerStruct_v1& next) const {
        next.value = value;
        next.extra = static_cast<float>(extra);
    }

    SS_COMPARISONS_MEMBER(InnerStruct_v2)
};

// Outer struct versioning
struct OuterStruct_v0
{
    InnerStruct_v0 inner;
    std::string name;

    auto ssTuple() const { return std::tie(inner, name); }
    SS_COMPARISONS_MEMBER(OuterStruct_v0)
};

struct OuterStruct_v1
{
    InnerStruct_v1 inner;  // Uses newer version of inner struct
    std::string name;

    auto ssTuple() const { return std::tie(inner, name); }
    SS_COMPARISONS_MEMBER(OuterStruct_v1)
};

struct OuterStruct_v2
{
    InnerStruct_v2 inner;  // Uses even newer version of inner struct
    std::string name;

    auto ssTuple() const { return std::tie(inner, name); }
    SS_COMPARISONS_MEMBER(OuterStruct_v2)
};

// Exception-throwing structs for testing error handling
struct ExceptionInnerStruct_v0
{
    int value {};

    using ssVersions = std::tuple<ExceptionInnerStruct_v0>;
    auto ssTuple() const { return std::tie(value); }
    SS_COMPARISONS_MEMBER(ExceptionInnerStruct_v0)
};

struct ExceptionInnerStruct_v1
{
    int value {};
    float extra {};

    using ssVersions = std::tuple<ExceptionInnerStruct_v0, ExceptionInnerStruct_v1>;
    auto ssTuple() const { return std::tie(value, extra); }

    void ssUpgradeFrom(const ExceptionInnerStruct_v0& /*prev*/) {
        throw std::runtime_error("Inner struct upgrade failed!");
    }

    void ssDowngradeTo(ExceptionInnerStruct_v0& /*next*/) const {
        throw std::runtime_error("Inner struct downgrade failed!");
    }

    SS_COMPARISONS_MEMBER(ExceptionInnerStruct_v1)
};

struct ExceptionOuterStruct_v0
{
    ExceptionInnerStruct_v0 inner;
    std::string name;

    auto ssTuple() const { return std::tie(inner, name); }
    SS_COMPARISONS_MEMBER(ExceptionOuterStruct_v0)
};

struct ExceptionOuterStruct_v1
{
    ExceptionInnerStruct_v1 inner;
    std::string name;

    auto ssTuple() const { return std::tie(inner, name); }
    SS_COMPARISONS_MEMBER(ExceptionOuterStruct_v1)
};

} // namespace

TEST(SuitableStruct, NestedVersioning_SaveLoadOuterStructV0)
{
    OuterStruct_v0 original;
    original.inner.value = 42;
    original.name = "test_v0";

    const auto saved = ssSave(original);
    OuterStruct_v0 loaded;
    ssLoad(saved, loaded);

    EXPECT_EQ(original, loaded);
}

TEST(SuitableStruct, NestedVersioning_SaveLoadOuterStructV1)
{
    OuterStruct_v1 original;
    original.inner.value = 42;
    original.inner.extra = 3.14f;
    original.name = "test_v1";

    const auto saved = ssSave(original);
    OuterStruct_v1 loaded;
    ssLoad(saved, loaded);

    EXPECT_EQ(original, loaded);
}

TEST(SuitableStruct, NestedVersioning_SaveLoadOuterStructV2)
{
    OuterStruct_v2 original;
    original.inner.value = 42;
    original.inner.extra = 3.14;
    original.inner.tag = "test_tag";
    original.name = "test_v2";

    const auto saved = ssSave(original);
    OuterStruct_v2 loaded;
    ssLoad(saved, loaded);

    EXPECT_EQ(original, loaded);
}

TEST(SuitableStruct, NestedVersioning_UpgradeOuterStructV0ToV1)
{
    OuterStruct_v0 original;
    original.inner.value = 42;
    original.name = "test_upgrade";

    const auto saved = ssSave(original);
    OuterStruct_v1 loaded;
    ssLoad(saved, loaded);

    EXPECT_EQ(loaded.inner.value, 42);
    EXPECT_FLOAT_EQ(loaded.inner.extra, 1.5f);  // Default value from upgrade
    EXPECT_EQ(loaded.name, "test_upgrade");
}

TEST(SuitableStruct, NestedVersioning_UpgradeOuterStructV1ToV2)
{
    OuterStruct_v1 original;
    original.inner.value = 42;
    original.inner.extra = 3.14f;
    original.name = "test_upgrade";

    const auto saved = ssSave(original);
    OuterStruct_v2 loaded;
    ssLoad(saved, loaded);

    EXPECT_EQ(loaded.inner.value, 42);
    EXPECT_FLOAT_EQ(loaded.inner.extra, 3.14f);  // Converted from double to float
    EXPECT_EQ(loaded.inner.tag, "upgraded");     // Default value from upgrade
    EXPECT_EQ(loaded.name, "test_upgrade");
}

TEST(SuitableStruct, NestedVersioning_DowngradeOuterStructV2ToV1)
{
    OuterStruct_v2 original;
    original.inner.value = 42;
    original.inner.extra = 3.14;
    original.inner.tag = "test_tag";
    original.name = "test_downgrade";

    const auto saved = ssSave(original);
    OuterStruct_v1 loaded;
    ssLoad(saved, loaded);

    EXPECT_EQ(loaded.inner.value, 42);
    EXPECT_FLOAT_EQ(loaded.inner.extra, 3.14f);  // Converted from double to float
    EXPECT_EQ(loaded.name, "test_downgrade");
}

TEST(SuitableStruct, NestedVersioning_DowngradeOuterStructV1ToV0)
{
    OuterStruct_v1 original;
    original.inner.value = 42;
    original.inner.extra = 3.14f;
    original.name = "test_downgrade";

    const auto saved = ssSave(original);
    OuterStruct_v0 loaded;
    ssLoad(saved, loaded);

    EXPECT_EQ(loaded.inner.value, 42);
    EXPECT_EQ(loaded.name, "test_downgrade");
}

TEST(SuitableStruct, NestedVersioning_UpgradeOuterStructV0ToV2)
{
    OuterStruct_v0 original;
    original.inner.value = 42;
    original.name = "test_upgrade";

    const auto saved = ssSave(original);
    OuterStruct_v2 loaded;
    ssLoad(saved, loaded);

    EXPECT_EQ(loaded.inner.value, 42);
    EXPECT_FLOAT_EQ(loaded.inner.extra, 1.5f);  // Default value from upgrade
    EXPECT_EQ(loaded.inner.tag, "upgraded");    // Default value from upgrade
    EXPECT_EQ(loaded.name, "test_upgrade");
}

TEST(SuitableStruct, NestedVersioning_DowngradeOuterStructV2ToV0)
{
    OuterStruct_v2 original;
    original.inner.value = 42;
    original.inner.extra = 3.14;
    original.inner.tag = "test_tag";
    original.name = "test_downgrade";

    const auto saved = ssSave(original);
    OuterStruct_v0 loaded;
    ssLoad(saved, loaded);

    EXPECT_EQ(loaded.inner.value, 42);
    EXPECT_EQ(loaded.name, "test_downgrade");
}

TEST(SuitableStruct, NestedVersioning_ExceptionInNestedUpgrade)
{
    ExceptionOuterStruct_v0 original;
    original.inner.value = 42;
    original.name = "test_exception";

    const auto saved = ssSave(original);
    ExceptionOuterStruct_v1 loaded;

    EXPECT_THROW(ssLoad(saved, loaded), std::runtime_error);
}

TEST(SuitableStruct, NestedVersioning_ExceptionInNestedDowngrade)
{
    ExceptionOuterStruct_v1 original;
    original.inner.value = 42;
    original.inner.extra = 3.14f;
    original.name = "test_exception";

    EXPECT_THROW(ssSave(original), std::runtime_error);
}
