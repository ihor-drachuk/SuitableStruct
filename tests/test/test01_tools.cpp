/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <SuitableStruct/Internals/LongSSO.h>
#include <string>
#include <utility>

using namespace SuitableStruct;

class LongSSOTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test data patterns
        shortData = "Hello";
        shortDataSize = 5;

        // Create data that exceeds SSO limit of 80 bytes
        longData = std::string(100, 'A');
        longDataSize = longData.size();

        // Medium data that fits in SSO
        mediumData = std::string(50, 'B');
        mediumDataSize = mediumData.size();
    }

    std::string shortData;
    size_t shortDataSize {};
    std::string longData;
    size_t longDataSize {};
    std::string mediumData;
    size_t mediumDataSize {};
};

// Test default constructor
TEST_F(LongSSOTest, DefaultConstructor) {
    LongSSO<> sso;

    EXPECT_EQ(sso.size(), 0);
    EXPECT_TRUE(sso.isShortBuf());
    EXPECT_FALSE(sso);  // operator bool
    EXPECT_EQ(sso.getSsoLimit(), 80);
}

// Test constructor with different SSO limits
TEST_F(LongSSOTest, TemplateParameter) {
    LongSSO<10> small;
    LongSSO<100> large;
    LongSSO<> defaultLimit;

    EXPECT_EQ(small.getSsoLimit(), 10);
    EXPECT_EQ(large.getSsoLimit(), 100);
    EXPECT_EQ(defaultLimit.getSsoLimit(), 80);
}

// Test constructor with external buffer
TEST_F(LongSSOTest, ExternalBufferConstructor) {
    ExternalSSOBuffer buffer;
    LongSSO<> sso(buffer);

    EXPECT_EQ(sso.size(), 0);
    EXPECT_TRUE(sso.isShortBuf());

    // Add data to trigger use of external buffer
    std::string largeData(100, 'X');
    sso.appendData(reinterpret_cast<const uint8_t*>(largeData.c_str()), largeData.size());

    EXPECT_FALSE(sso.isShortBuf());
    EXPECT_EQ(sso.size(), 100);
    EXPECT_EQ(buffer.size(), 100);
}

// Test copy constructor
TEST_F(LongSSOTest, CopyConstructor) {
    // Test copying short buffer
    LongSSO<> original;
    original.appendData(reinterpret_cast<const uint8_t*>(shortData.c_str()), shortDataSize);

    LongSSO<> copy(original);
    EXPECT_EQ(copy.size(), shortDataSize);
    EXPECT_TRUE(copy.isShortBuf());
    EXPECT_EQ(memcmp(copy.data(), original.data(), shortDataSize), 0);

    // Test copying long buffer
    LongSSO<> originalLong;
    originalLong.appendData(reinterpret_cast<const uint8_t*>(longData.c_str()), longDataSize);

    LongSSO<> copyLong(originalLong);
    EXPECT_EQ(copyLong.size(), longDataSize);
    EXPECT_FALSE(copyLong.isShortBuf());
    EXPECT_EQ(memcmp(copyLong.data(), originalLong.data(), longDataSize), 0);
}

// Test move constructor
TEST_F(LongSSOTest, MoveConstructor) {
    // Test moving short buffer
    LongSSO<> original;
    original.appendData(reinterpret_cast<const uint8_t*>(shortData.c_str()), shortDataSize);

    LongSSO<> moveCtor(std::move(original));
    EXPECT_EQ(moveCtor.size(), shortDataSize);
    EXPECT_TRUE(moveCtor.isShortBuf());
    EXPECT_EQ(memcmp(moveCtor.data(), shortData.c_str(), shortDataSize), 0);

    // Test moving long buffer
    LongSSO<> originalLong;
    originalLong.appendData(reinterpret_cast<const uint8_t*>(longData.c_str()), longDataSize);
    auto* originalPtr = originalLong.data();

    LongSSO<> moveCtorLong(std::move(originalLong));
    EXPECT_EQ(moveCtorLong.size(), longDataSize);
    EXPECT_FALSE(moveCtorLong.isShortBuf());
    EXPECT_EQ(moveCtorLong.data(), originalPtr);  // Should be same memory
}

// Test copy assignment operator
TEST_F(LongSSOTest, CopyAssignment) {
    LongSSO<> source, target;

    // Self-assignment
    source.appendData(reinterpret_cast<const uint8_t*>(shortData.c_str()), shortDataSize);
    source = source;
    EXPECT_EQ(source.size(), shortDataSize);

    // Copy short to empty
    target = source;
    EXPECT_EQ(target.size(), shortDataSize);
    EXPECT_EQ(memcmp(target.data(), source.data(), shortDataSize), 0);

    // Copy long to short
    LongSSO<> longSource;
    longSource.appendData(reinterpret_cast<const uint8_t*>(longData.c_str()), longDataSize);
    target = longSource;
    EXPECT_EQ(target.size(), longDataSize);
    EXPECT_FALSE(target.isShortBuf());
    EXPECT_EQ(memcmp(target.data(), longSource.data(), longDataSize), 0);
}

// Test move assignment operator
TEST_F(LongSSOTest, MoveAssignment) {
    LongSSO<> source, target;

    // Self-assignment
    source.appendData(reinterpret_cast<const uint8_t*>(shortData.c_str()), shortDataSize);
    source = std::move(source);
    EXPECT_EQ(source.size(), shortDataSize);

    // Move short to empty
    LongSSO<> shortSource;
    shortSource.appendData(reinterpret_cast<const uint8_t*>(shortData.c_str()), shortDataSize);
    target = std::move(shortSource);
    EXPECT_EQ(target.size(), shortDataSize);
    EXPECT_EQ(memcmp(target.data(), shortData.c_str(), shortDataSize), 0);

    // Move long to target
    LongSSO<> longSource;
    longSource.appendData(reinterpret_cast<const uint8_t*>(longData.c_str()), longDataSize);
    auto* originalPtr = longSource.data();

    target = std::move(longSource);
    EXPECT_EQ(target.size(), longDataSize);
    EXPECT_FALSE(target.isShortBuf());
    EXPECT_EQ(target.data(), originalPtr);
}

// Test equality operators
TEST_F(LongSSOTest, EqualityOperators) {
    LongSSO<> a, b;
    LongSSO<50> c;

    // Empty objects
    EXPECT_EQ(a, b);
    EXPECT_EQ(a, c);
    EXPECT_FALSE(a != b);

    // Same data, different SSO limits
    a.appendData(reinterpret_cast<const uint8_t*>(shortData.c_str()), shortDataSize);
    c.appendData(reinterpret_cast<const uint8_t*>(shortData.c_str()), shortDataSize);
    EXPECT_EQ(a, c);
    EXPECT_NE(a, b);

    // Different data
    b.appendData(reinterpret_cast<const uint8_t*>("World"), 5);
    EXPECT_NE(a, b);

    // Same data, one short one long
    LongSSO<5> small;  // Forces long buffer for short data
    small.appendData(reinterpret_cast<const uint8_t*>(shortData.c_str()), shortDataSize);
    EXPECT_EQ(a, small);  // Content should be same despite different storage
}

// Test bool operator
TEST_F(LongSSOTest, BoolOperator) {
    LongSSO<> sso;
    EXPECT_FALSE(sso);

    sso.appendData(reinterpret_cast<const uint8_t*>("a"), 1);
    EXPECT_TRUE(sso);

    sso.clear();
    EXPECT_FALSE(sso);
}

// Test data access methods
TEST_F(LongSSOTest, DataAccess) {
    LongSSO<> sso;

    // Empty state
    EXPECT_EQ(sso.size(), 0);
    EXPECT_NE(sso.data(), nullptr);
    EXPECT_NE(sso.cdata(), nullptr);
    EXPECT_EQ(sso.data(), sso.cdata());

    // With data
    sso.appendData(reinterpret_cast<const uint8_t*>(shortData.c_str()), shortDataSize);
    EXPECT_EQ(sso.size(), shortDataSize);
    EXPECT_EQ(memcmp(sso.data(), shortData.c_str(), shortDataSize), 0);
    EXPECT_EQ(memcmp(sso.cdata(), shortData.c_str(), shortDataSize), 0);

    // Modify through data()
    sso.data()[0] = 'h';  // Hello -> hello
    EXPECT_EQ(sso.data()[0], 'h');
}

// Test appendData functionality
TEST_F(LongSSOTest, AppendData) {
    LongSSO<> sso;

    // Append to empty
    sso.appendData(reinterpret_cast<const uint8_t*>(shortData.c_str()), shortDataSize);
    EXPECT_EQ(sso.size(), shortDataSize);
    EXPECT_TRUE(sso.isShortBuf());
    EXPECT_EQ(memcmp(sso.data(), shortData.c_str(), shortDataSize), 0);

    // Append more (still short)
    sso.appendData(reinterpret_cast<const uint8_t*>(" World"), 6);
    EXPECT_EQ(sso.size(), 11);
    EXPECT_TRUE(sso.isShortBuf());
    EXPECT_EQ(std::string(reinterpret_cast<const char*>(sso.data()), sso.size()), "Hello World");

    // Append enough to trigger long buffer
    const std::string padding(80, 'X');
    sso.appendData(reinterpret_cast<const uint8_t*>(padding.c_str()), padding.size());
    EXPECT_FALSE(sso.isShortBuf());
    EXPECT_EQ(sso.size(), 91);
}

// Test allocate_copy functionality
TEST_F(LongSSOTest, AllocateCopy) {
    LongSSO<> sso;

    // Allocate without copying
    uint8_t* ptr1 = sso.allocate_copy(10);
    EXPECT_NE(ptr1, nullptr);
    EXPECT_EQ(sso.size(), 10);
    EXPECT_TRUE(sso.isShortBuf());

    // Fill the allocated space
    memset(ptr1, 'A', 10);
    EXPECT_EQ(sso.data()[0], 'A');

    // Allocate with copying
    const std::string testData = "TestData";
    uint8_t* ptr2 = sso.allocate_copy(testData.size(),
                                      reinterpret_cast<const uint8_t*>(testData.c_str()));
    EXPECT_EQ(sso.size(), 18);
    EXPECT_EQ(memcmp(ptr2, testData.c_str(), testData.size()), 0);

    // Allocate large amount to trigger long buffer
    sso.allocate_copy(100);
    EXPECT_FALSE(sso.isShortBuf());
    EXPECT_EQ(sso.size(), 118);
}

// Test reduceSize functionality
TEST_F(LongSSOTest, ReduceSize) {
    LongSSO<> sso;

    // Add data
    sso.appendData(reinterpret_cast<const uint8_t*>(mediumData.c_str()), mediumDataSize);
    EXPECT_EQ(sso.size(), mediumDataSize);

    // Reduce size
    sso.reduceSize(10);
    EXPECT_EQ(sso.size(), mediumDataSize - 10);
    EXPECT_TRUE(sso.isShortBuf());

    // Test with long buffer
    sso.appendData(reinterpret_cast<const uint8_t*>(longData.c_str()), longDataSize);
    EXPECT_FALSE(sso.isShortBuf());
    size_t sizeBefore = sso.size();

    sso.reduceSize(20);
    EXPECT_EQ(sso.size(), sizeBefore - 20);
    EXPECT_FALSE(sso.isShortBuf());

    // Reduce to zero
    sso.reduceSize(sso.size());
    EXPECT_EQ(sso.size(), 0);
    EXPECT_FALSE(sso.isShortBuf());  // Stays in long mode
}

// Test clear functionality
TEST_F(LongSSOTest, Clear) {
    LongSSO<> sso;

    // Clear empty
    sso.clear();
    EXPECT_EQ(sso.size(), 0);
    EXPECT_TRUE(sso.isShortBuf());

    // Clear short buffer
    sso.appendData(reinterpret_cast<const uint8_t*>(shortData.c_str()), shortDataSize);
    sso.clear();
    EXPECT_EQ(sso.size(), 0);
    EXPECT_TRUE(sso.isShortBuf());

    // Clear long buffer
    sso.appendData(reinterpret_cast<const uint8_t*>(longData.c_str()), longDataSize);
    EXPECT_FALSE(sso.isShortBuf());
    sso.clear();
    EXPECT_EQ(sso.size(), 0);
    EXPECT_TRUE(sso.isShortBuf());  // Should revert to short mode
}

// Test transition from short to long buffer
TEST_F(LongSSOTest, ShortToLongTransition) {
    LongSSO<20> sso;  // Small SSO limit for easy testing

    // Fill up to limit
    const std::string data19(19, 'A');
    sso.appendData(reinterpret_cast<const uint8_t*>(data19.c_str()), data19.size());
    EXPECT_TRUE(sso.isShortBuf());
    EXPECT_EQ(sso.size(), 19);

    // One more byte to reach limit
    sso.appendData(reinterpret_cast<const uint8_t*>("B"), 1);
    EXPECT_TRUE(sso.isShortBuf());
    EXPECT_EQ(sso.size(), 20);

    // One more byte to exceed limit
    sso.appendData(reinterpret_cast<const uint8_t*>("C"), 1);
    EXPECT_FALSE(sso.isShortBuf());
    EXPECT_EQ(sso.size(), 21);

    // Verify data integrity
    const std::string expected = data19 + "BC";
    EXPECT_EQ(std::string(reinterpret_cast<const char*>(sso.data()), sso.size()), expected);
}

// Test memory management and external buffer usage
TEST_F(LongSSOTest, ExternalBufferUsage) {
    ExternalSSOBuffer externalBuffer;
    LongSSO<10> sso(externalBuffer);

    // Add data that exceeds SSO limit
    std::string testData(20, 'X');
    sso.appendData(reinterpret_cast<const uint8_t*>(testData.c_str()), testData.size());

    EXPECT_FALSE(sso.isShortBuf());
    EXPECT_EQ(sso.size(), 20);
    EXPECT_EQ(externalBuffer.size(), 20);

    // Verify the external buffer contains the data
    EXPECT_EQ(memcmp(externalBuffer.data(), testData.c_str(), testData.size()), 0);
    EXPECT_EQ(sso.data(), externalBuffer.data());  // Should point to same memory
}

// Test different SSO limits working together
TEST_F(LongSSOTest, DifferentSSOLimits) {
    LongSSO<10> small;
    LongSSO<50> medium;
    LongSSO<100> large;

    const std::string data30(30, 'A');

    small.appendData(reinterpret_cast<const uint8_t*>(data30.c_str()), data30.size());
    medium.appendData(reinterpret_cast<const uint8_t*>(data30.c_str()), data30.size());
    large.appendData(reinterpret_cast<const uint8_t*>(data30.c_str()), data30.size());

    EXPECT_FALSE(small.isShortBuf());   // 30 > 10
    EXPECT_TRUE(medium.isShortBuf());   // 30 <= 50
    EXPECT_TRUE(large.isShortBuf());    // 30 <= 100

    // All should be equal despite different storage
    EXPECT_EQ(small, medium);
    EXPECT_EQ(medium, large);
    EXPECT_EQ(small, large);
}

// Test edge cases and boundary conditions
TEST_F(LongSSOTest, EdgeCases) {
    // Zero-sized operations
    LongSSO<> sso;
    sso.appendData(nullptr, 0);
    EXPECT_EQ(sso.size(), 0);

    uint8_t* ptr = sso.allocate_copy(0, nullptr);
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(sso.size(), 0);

    // Reduce size by 0
    sso.appendData(reinterpret_cast<const uint8_t*>("test"), 4);
    size_t sizeBefore = sso.size();
    sso.reduceSize(0);
    EXPECT_EQ(sso.size(), sizeBefore);

    // Multiple clears
    sso.clear();
    sso.clear();
    EXPECT_EQ(sso.size(), 0);
    EXPECT_TRUE(sso.isShortBuf());
}

// Test large data handling
TEST_F(LongSSOTest, LargeDataHandling) {
    LongSSO<> sso;

    // Very large allocation
    constexpr size_t largeSize = 10000;
    uint8_t* ptr = sso.allocate_copy(largeSize);
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(sso.size(), largeSize);
    EXPECT_FALSE(sso.isShortBuf());

    // Fill with pattern
    for (size_t i = 0; i < largeSize; ++i) {
        ptr[i] = static_cast<uint8_t>(i % 256);
    }

    // Verify pattern
    for (size_t i = 0; i < largeSize; ++i) {
        EXPECT_EQ(sso.data()[i], static_cast<uint8_t>(i % 256));
    }

    // Reduce to smaller size
    sso.reduceSize(largeSize - 100);
    EXPECT_EQ(sso.size(), 100);

    // Verify remaining data
    for (size_t i = 0; i < 100; ++i) {
        EXPECT_EQ(sso.data()[i], static_cast<uint8_t>(i % 256));
    }
}

// Test copy/move semantics with external buffers
TEST_F(LongSSOTest, ExternalBufferCopyMove) {
    ExternalSSOBuffer buffer1, buffer2;

    {
        LongSSO<5> sso1(buffer1);
        LongSSO<5> sso2(buffer2);

        // Add data to trigger external buffer usage
        std::string data(10, 'A');
        sso1.appendData(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());

        EXPECT_EQ(buffer1.size(), 10);
        EXPECT_EQ(buffer2.size(), 0);

        // Copy should create new internal buffer, not use external
        sso2 = sso1;
        EXPECT_EQ(sso2.size(), 10);
        EXPECT_EQ(buffer2.size(), 0);  // External buffer not used for copy

        // Move should transfer ownership
        LongSSO<5> sso3;
        sso3 = std::move(sso1);
        EXPECT_EQ(sso3.size(), 10);
        // sso1 should be in valid but unspecified state
    }

    // Buffers should still be valid after SSO destruction
    EXPECT_EQ(buffer1.size(), 10);
    EXPECT_EQ(buffer2.size(), 0);
}

// Performance and stress test
TEST_F(LongSSOTest, StressTest) {
    LongSSO<> sso;

    // Incremental growth
    constexpr size_t iterations = 1000;
    for (size_t i = 0; i < iterations; ++i) {
        const auto byte = static_cast<uint8_t>(i % 256);
        sso.appendData(&byte, 1);
        EXPECT_EQ(sso.size(), i + 1);
        EXPECT_EQ(sso.data()[i], byte);
    }

    // Verify all data
    for (size_t i = 0; i < iterations; ++i) {
        EXPECT_EQ(sso.data()[i], static_cast<uint8_t>(i % 256));
    }

    // Incremental reduction
    for (size_t i = iterations; i > 0; --i) {
        sso.reduceSize(1);
        EXPECT_EQ(sso.size(), i - 1);
    }

    EXPECT_EQ(sso.size(), 0);
    EXPECT_FALSE(sso);
}
