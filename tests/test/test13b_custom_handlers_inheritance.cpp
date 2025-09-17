/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>

#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Containers/vector.h>

using namespace SuitableStruct;

namespace {

struct Data
{
    int a {};
    std::string b;

    auto ssTuple() const { return std::tie(a, b); }
    SS_COMPARISONS_MEMBER(Data)
};

using Struct_v0 = std::vector<Data>;

struct Struct_v0_b : public std::vector<Data>
{
    using std::vector<Data>::vector;
    Buffer ssSaveImpl() const {
        return ::ssSaveImpl(static_cast<const std::vector<Data>&>(*this));
    }

    void ssLoadImpl(BufferReader& src) {
        ::ssLoadImpl(src, static_cast<std::vector<Data>&>(*this));
    }

    bool operator==(const Struct_v0_b& rhs) const {
        return static_cast<const std::vector<Data>&>(*this) ==
               static_cast<const std::vector<Data>&>(rhs);
    }
};

struct Struct_v0_c : public std::vector<Data>
{
    using std::vector<Data>::vector;
    auto ssTuple() const { return std::tie(static_cast<const std::vector<Data>&>(*this)); }
    SS_COMPARISONS_MEMBER(Struct_v0_c)
};

} // namespace

TEST(SuitableStruct, CustomHandlers_Inheritance)
{
    Struct_v0 original { {1, "one"}, {2, "two"}, {3, "three"} };;
    const auto buffer = SuitableStruct::ssSave(original);
    const auto restored = SuitableStruct::ssLoadRet<Struct_v0>(buffer);
    ASSERT_EQ(original, restored);

    const auto restored_b = SuitableStruct::ssLoadRet<Struct_v0_b>(buffer);
    ASSERT_EQ(original, restored_b);

    // Won't work.
    //const auto restored_c = SuitableStruct::ssLoadRet<Struct_v0_c>(buffer);
    //ASSERT_EQ(original, restored_c);
}
