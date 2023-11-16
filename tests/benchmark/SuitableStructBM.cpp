/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <benchmark/benchmark.h>

BENCHMARK_MAIN();

#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Containers/vector.h>
#include <SuitableStruct/Containers/list.h>
#include <SuitableStruct/Containers/array.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <SuitableStruct/SerializerJson.h>
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY

namespace {

struct Struct1
{
    short a{};
    long b{};
    double c{};
    std::string d;
    std::vector<int> e;         // #include <SuitableStruct/Containers/vector.h>
    std::list<std::string> f;   // #include <SuitableStruct/Containers/list.h>
    std::array<double, 5> g{};  // #include <SuitableStruct/Containers/array.h>
    std::tuple<int, std::string> h {};
    std::shared_ptr<int> i;
    std::unique_ptr<int> j;
    std::optional<int> k1, k2;

    auto ssTuple() const { return std::tie(a, b, c, d, e, f, g, h, i, j, k1, k2); }
    auto ssNamesTuple() const { return std::tie("a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k1", "k2"); }
    SS_COMPARISONS_MEMBER_ONLY_EQ(Struct1);
};

} // namespace

static void StubBenchmark(benchmark::State& state)
{
    int i = 0;

    while (state.KeepRunning())
        i++;

    (void)i;
}

BENCHMARK(StubBenchmark);


static void serialization_raw(benchmark::State& state)
{
    const Struct1 value;
    while (state.KeepRunning()) {
        const auto result = SuitableStruct::ssSave(value);
        (void)result;
    }
}

BENCHMARK(serialization_raw);


#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
static void serialization_json(benchmark::State& state)
{
    const Struct1 value;
    while (state.KeepRunning()) {
        const auto result = SuitableStruct::ssJsonSave(value);
        (void)result;
    }
}

BENCHMARK(serialization_json);
#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
