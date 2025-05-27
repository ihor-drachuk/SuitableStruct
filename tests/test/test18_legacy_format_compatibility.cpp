/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <SuitableStruct/Serializer.h>
#include <SuitableStruct/Comparisons.h>
#include <SuitableStruct/Hashes.h>
#include <SuitableStruct/Exceptions.h>

using namespace SuitableStruct;

namespace {

// Test structure with 3 versions for legacy format compatibility testing
struct TestStruct_v0
{
    int value {};

    using ssVersions = std::tuple<TestStruct_v0>;
    auto ssTuple() const { return std::tie(value); }
    SS_COMPARISONS_MEMBER(TestStruct_v0)

    Buffer serializeLegacy() const {
        Buffer result;
        result.write(value);
        return result;
    }
};

struct TestStruct_v1
{
    int value {};
    float extra {};

    using ssVersions = std::tuple<TestStruct_v0, TestStruct_v1>;
    auto ssTuple() const { return std::tie(value, extra); }

    void ssUpgradeFrom(const TestStruct_v0& prev) {
        value = prev.value;
        extra = 2.5f;
    }

    void ssDowngradeTo(TestStruct_v0& next) const {
        next.value = value;
    }

    SS_COMPARISONS_MEMBER(TestStruct_v1)

    Buffer serializeLegacy() const {
        Buffer result;
        result.write(value);
        result.write(extra);
        return result;
    }
};

struct TestStruct_v2
{
    int value {};
    double extra {};
    std::string name;

    using ssVersions = std::tuple<TestStruct_v0, TestStruct_v1, TestStruct_v2>;
    auto ssTuple() const { return std::tie(value, extra, name); }

    void ssUpgradeFrom(const TestStruct_v1& prev) {
        value = prev.value;
        extra = static_cast<double>(prev.extra);
        name = "upgraded_v2";
    }

    void ssDowngradeTo(TestStruct_v1& next) const {
        next.value = value;
        next.extra = static_cast<float>(extra);
    }

    SS_COMPARISONS_MEMBER(TestStruct_v2)

    Buffer serializeLegacy() const {
        Buffer result;
        result.write(value);
        result.write(extra);
        result.write(static_cast<uint8_t>(0)); // All classes (even std::string) use versions.
        result.write(static_cast<uint64_t>(name.size()));
        result.writeRaw(name.data(), name.size());
        return result;
    }
};

// Helper function to create legacy F0 format buffer manually
template<typename T>
Buffer createLegacyF0Buffer(const T& obj, uint8_t version)
{
    Buffer payload;
    payload.writeRaw(static_cast<const void*>(Internal::SS_FORMAT_F0), sizeof(Internal::SS_FORMAT_F0));
    payload.write(version);
    payload += obj.serializeLegacy();

    Buffer header;
    header.write(static_cast<uint64_t>(payload.size()));
    header.write(ssHashRaw_F0(payload.data(), payload.size()));

    return header + payload;
}

} // namespace

TEST(SuitableStruct, LegacyFormatCompatibility_LoadTestStructV0_SameVersion)
{
    TestStruct_v0 original;
    original.value = 42;

    auto legacyBuffer = createLegacyF0Buffer(original, 0);

    TestStruct_v0 loaded;
    ssLoad(legacyBuffer, loaded);

    EXPECT_EQ(original, loaded);
}

TEST(SuitableStruct, LegacyFormatCompatibility_LoadTestStructV1_SameVersion)
{
    TestStruct_v1 original;
    original.value = 42;
    original.extra = 3.14f;

    auto legacyBuffer = createLegacyF0Buffer(original, 1);

    TestStruct_v1 loaded;
    ssLoad(legacyBuffer, loaded);

    EXPECT_EQ(original, loaded);
}

TEST(SuitableStruct, LegacyFormatCompatibility_LoadTestStructV2_SameVersion)
{
    TestStruct_v2 original;
    original.value = 42;
    original.extra = 3.14;
    original.name = "test_v2";

    auto legacyBuffer = createLegacyF0Buffer(original, 2);

    TestStruct_v2 loaded;
    ssLoad(legacyBuffer, loaded);

    EXPECT_EQ(original, loaded);
}

TEST(SuitableStruct, LegacyFormatCompatibility_UpgradeV0ToV1)
{
    // Create v0 struct and save it in legacy format
    TestStruct_v0 original;
    original.value = 100;

    auto legacyBuffer = createLegacyF0Buffer(original, 0);

    // Load it as v1 (should trigger upgrade)
    TestStruct_v1 loaded;
    ssLoad(legacyBuffer, loaded);

    EXPECT_EQ(loaded.value, 100);
    EXPECT_FLOAT_EQ(loaded.extra, 2.5f);  // Default value from upgrade
}

TEST(SuitableStruct, LegacyFormatCompatibility_UpgradeV0ToV2)
{
    // Create v0 struct and save it in legacy format
    TestStruct_v0 original;
    original.value = 200;

    auto legacyBuffer = createLegacyF0Buffer(original, 0);

    // Load it as v2 (should trigger upgrade v0->v1->v2)
    TestStruct_v2 loaded;
    ssLoad(legacyBuffer, loaded);

    EXPECT_EQ(loaded.value, 200);
    EXPECT_DOUBLE_EQ(loaded.extra, 2.5);    // Upgraded from v1's default
    EXPECT_EQ(loaded.name, "upgraded_v2");  // Default value from v1->v2 upgrade
}

TEST(SuitableStruct, LegacyFormatCompatibility_UpgradeV1ToV2)
{
    // Create v1 struct and save it in legacy format
    TestStruct_v1 original;
    original.value = 300;
    original.extra = 7.5f;

    auto legacyBuffer = createLegacyF0Buffer(original, 1);

    // Load it as v2 (should trigger upgrade)
    TestStruct_v2 loaded;
    ssLoad(legacyBuffer, loaded);

    EXPECT_EQ(loaded.value, 300);
    EXPECT_DOUBLE_EQ(loaded.extra, 7.5);     // Converted from float to double
    EXPECT_EQ(loaded.name, "upgraded_v2");   // Default value from upgrade
}

TEST(SuitableStruct, LegacyFormatCompatibility_DowngradeV2ToV1)
{
    // Create v2 struct and save it in legacy format
    TestStruct_v2 original;
    original.value = 400;
    original.extra = 9.75;
    original.name = "test_downgrade";

    auto legacyBuffer = createLegacyF0Buffer(original, 2);

    // Legacy format doesn't support downgrades - should throw
    TestStruct_v1 loaded;
    EXPECT_THROW(ssLoad(legacyBuffer, loaded), VersionError);
}

TEST(SuitableStruct, LegacyFormatCompatibility_DowngradeV2ToV0)
{
    // Create v2 struct and save it in legacy format
    TestStruct_v2 original;
    original.value = 500;
    original.extra = 12.25;
    original.name = "test_downgrade_v0";

    auto legacyBuffer = createLegacyF0Buffer(original, 2);

    // Legacy format doesn't support downgrades - should throw
    TestStruct_v0 loaded;
    EXPECT_THROW(ssLoad(legacyBuffer, loaded), VersionError);
}

TEST(SuitableStruct, LegacyFormatCompatibility_DowngradeV1ToV0)
{
    // Create v1 struct and save it in legacy format
    TestStruct_v1 original;
    original.value = 600;
    original.extra = 15.5f;

    auto legacyBuffer = createLegacyF0Buffer(original, 1);

    // Legacy format doesn't support downgrades - should throw
    TestStruct_v0 loaded;
    EXPECT_THROW(ssLoad(legacyBuffer, loaded), VersionError);
}

TEST(SuitableStruct, LegacyFormatCompatibility_InvalidVersionShouldThrow)
{
    // Create v0 struct but mark it with invalid version
    TestStruct_v0 original;
    original.value = 42;

    auto legacyBuffer = createLegacyF0Buffer(original, 99);  // Invalid version

    // Should throw when trying to load
    TestStruct_v0 loaded;
    EXPECT_THROW(ssLoad(legacyBuffer, loaded), VersionError);
}

TEST(SuitableStruct, LegacyFormatCompatibility_CorruptedLegacyBufferShouldThrow)
{
    // Use more complex structure with more data
    TestStruct_v2 originalData;
    originalData.value = 42;
    originalData.extra = 3.14159;
    originalData.name = "test_corruption_data_string";

    const auto referenceBuffer = createLegacyF0Buffer(originalData, 2);
    const size_t bufferLen = referenceBuffer.size();

    // Test corruption of each byte in the buffer.
    // We skip the last 'bytesToSkipAtEnd' bytes. This is based on the original test's
    // understanding that ssHashRaw_F0 (legacy hash) might not correctly cover these
    // trailing bytes of the payload, so corrupting them might not throw an IntegrityError.
    // This loop tests that all *other* bytes, which are expected to be covered, do trigger an error.
    constexpr size_t bytesToSkipAtEnd = 3; // Value from the original test's logic
    size_t corruptionLoopEnd = 0;
    if (bufferLen > bytesToSkipAtEnd) {
        corruptionLoopEnd = bufferLen - bytesToSkipAtEnd;
    }

    for (size_t i = 0; i < corruptionLoopEnd; ++i) {
        SCOPED_TRACE("Corrupting byte at index " + std::to_string(i) + " of " + std::to_string(bufferLen) +
                     " (skipping last " + std::to_string(bytesToSkipAtEnd) + " bytes of entire buffer)");

        auto corruptedBuffer = referenceBuffer;
        ASSERT_LT(i, corruptedBuffer.size()); // Ensure index is valid
        corruptedBuffer.data()[i]++;

        TestStruct_v2 loadedStruct;
        try {
            ssLoad(corruptedBuffer, loadedStruct);
            ADD_FAILURE() << "Expected IntegrityError but no exception was thrown. "
                          << "Corrupted byte at index " << i;
        } catch (const IntegrityError&) {
            // Expected - test passes for this iteration
        } catch (const std::exception& e) {
            ADD_FAILURE() << "Expected IntegrityError but got different exception: " << e.what()
                          << ". Corrupted byte at index " << i;
        }
    }

    // Test various buffer truncations
    for (size_t r = 1; r < bufferLen; ++r) { // r is reduction amount
        SCOPED_TRACE("Truncating buffer by " + std::to_string(r) + " bytes. Original size: " + std::to_string(bufferLen));

        auto truncatedBuffer = referenceBuffer;
        truncatedBuffer.reduceSize(r);

        TestStruct_v2 loadedStruct;
        try {
            ssLoad(truncatedBuffer, loadedStruct);
            ADD_FAILURE() << "Expected IntegrityError or std::out_of_range on truncated buffer but no exception was thrown. "
                          << "Truncated by " << r << " bytes.";
        } catch (const IntegrityError&) {
            // Expected due to hash mismatch or size mismatch if enough data remains for hash check
        } catch (const std::out_of_range&) {
            // Also expected if truncation causes an attempt to read past the end
        } catch (const std::exception& e) {
            ADD_FAILURE() << "Expected IntegrityError or std::out_of_range but got different exception: " << e.what()
                          << ". Truncated by " << r << " bytes.";
        }
    }

    // Specific corruption scenarios (these are targeted and valuable)
    {
        constexpr size_t HeaderSize = sizeof(uint64_t) + sizeof(uint32_t);
        constexpr size_t SizeOffset = 0;
        constexpr size_t HashOffset = sizeof(uint64_t);
        constexpr size_t FormatMarkerOffset = HeaderSize;
        constexpr size_t VersionOffset = FormatMarkerOffset + sizeof(Internal::SS_FORMAT_F0);

        const auto specificTestBaseBuffer = createLegacyF0Buffer(originalData, 2);
        TestStruct_v2 loadedSpecificStruct;

        auto runSpecificCorruptionTest =
            [&](size_t offset, const std::string& corruptionType) {
            SCOPED_TRACE("Corrupting " + corruptionType);
            auto bufferToCorrupt = specificTestBaseBuffer;
            ASSERT_LT(offset, bufferToCorrupt.size()) << "Offset out of bounds for " << corruptionType;
            bufferToCorrupt.data()[offset]++;
            try {
                ssLoad(bufferToCorrupt, loadedSpecificStruct);
                ADD_FAILURE() << "Expected IntegrityError for " << corruptionType << " but no exception was thrown.";
            } catch (const IntegrityError&) {
                // Expected
            } catch (const std::exception& e) {
                ADD_FAILURE() << "Expected IntegrityError for " << corruptionType
                              << " but got different exception: " << e.what();
            }
        };

        runSpecificCorruptionTest(FormatMarkerOffset, "format marker");
        runSpecificCorruptionTest(SizeOffset, "size field");
        runSpecificCorruptionTest(HashOffset, "hash");
        runSpecificCorruptionTest(VersionOffset, "version field");
    }
}
