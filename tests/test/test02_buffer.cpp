/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <gtest/gtest.h>
#include <stdexcept>
#include <SuitableStruct/Buffer.h>
#include <SuitableStruct/BufferReader.h>

using namespace SuitableStruct;

template<size_t Size>
void bufferTester()
{
    constexpr size_t dataSz = Size;
    char data[dataSz];
    const char strHello[] = "Hello";
    const char strWorld[] = "world";
    constexpr auto strHelloSz = sizeof(strHello) - 1;
    constexpr auto strWorldSz = sizeof(strWorld) - 1;

    memset(data, 1, dataSz);
    memcpy(data, strHello, strHelloSz);
    memcpy(data + dataSz - strWorldSz, strWorld, strWorldSz);

    Buffer buffer;
    BufferReader reader(buffer);
    buffer.writeRaw(strHello, strHelloSz);

    Buffer midPart(dataSz - strHelloSz - strWorldSz);
    memset(midPart.data(), 1, midPart.size());
    buffer += midPart;

    buffer.writeRaw(strWorld, strWorldSz);

    ASSERT_EQ(buffer.size(), dataSz);
    ASSERT_EQ(memcmp(buffer.cdata(), data, dataSz), 0);

    ASSERT_EQ(buffer.size(), reader.size());
    char data2[dataSz];
    constexpr auto halfSz = dataSz / 2;
    constexpr auto restSz = dataSz - halfSz;
    reader.readRaw(data2, halfSz);
    reader.readRaw(data2 + halfSz, restSz);
    ASSERT_EQ(memcmp(data, data2, dataSz), 0);

    ASSERT_EQ(reader.size(), dataSz);
    ASSERT_EQ(reader.position(), dataSz);
    ASSERT_EQ(reader.rest(), 0);
    ASSERT_THROW(reader.readRaw(data2, 1), std::out_of_range);
}

TEST(SuitableStruct, BufferTest)
{
    bufferTester<LongSSO<>::getSsoLimit() / 2>();
    bufferTester<LongSSO<>::getSsoLimit() - 1>();
    bufferTester<LongSSO<>::getSsoLimit()>();
    bufferTester<LongSSO<>::getSsoLimit() + 1>();
    bufferTester<LongSSO<>::getSsoLimit() * 2>();
    bufferTester<10>();
}

TEST(SuitableStruct, BufferTest_getBuffer)
{
    const char strHello[] = "Hello";
    Buffer buf(strHello, 5);
    BufferReader reader(buf, 1);

    char tmp;
    reader.read(tmp);
    ASSERT_EQ(tmp, 'e');

    const auto buf1 = reader.bufferSrc();
    const auto buf2 = reader.bufferMapped();
    const auto buf3 = reader.bufferRest();

    ASSERT_EQ(buf1, Buffer::fromConstChar("Hello"));
    ASSERT_EQ(buf2, Buffer::fromConstChar("ello"));
    ASSERT_EQ(buf3, Buffer::fromConstChar("llo"));
}

TEST(SuitableStruct, BufferReader_Advance)
{
    const char strHello[] = "Hello";
    Buffer buf(strHello, 5);
    BufferReader reader(buf);

    // Test advancing to negative position from start (should throw)
    ASSERT_EQ(reader.position(), 0);
    ASSERT_THROW(reader.advance(-1), std::out_of_range);
    ASSERT_EQ(reader.position(), 0); // Position should remain unchanged after failed advance

    // Advance to middle position
    reader.advance(2);
    ASSERT_EQ(reader.position(), 2);

    // Valid negative advance
    reader.advance(-1);
    ASSERT_EQ(reader.position(), 1);

    // Try to advance beyond beginning (should throw)
    ASSERT_THROW(reader.advance(-2), std::out_of_range);
    ASSERT_EQ(reader.position(), 1); // Position should remain unchanged after failed advance

    // Advance to end
    reader.advance(4);
    ASSERT_EQ(reader.position(), 5);

    // Try to advance beyond end (should throw)
    ASSERT_THROW(reader.advance(1), std::out_of_range);
    ASSERT_EQ(reader.position(), 5); // Position should remain unchanged after failed advance
}

TEST(SuitableStruct, BufferReader_SeekBeyondBounds)
{
    const char strHello[] = "Hello";
    Buffer buf(strHello, 5);
    BufferReader reader(buf);

    // Valid seeks
    ASSERT_EQ(reader.seek(0), 0);
    ASSERT_EQ(reader.position(), 0);

    ASSERT_EQ(reader.seek(3), 3);
    ASSERT_EQ(reader.position(), 3);

    ASSERT_EQ(reader.seek(5), 5); // Seek to end is valid
    ASSERT_EQ(reader.position(), 5);

    // Invalid seek beyond bounds (should throw)
    ASSERT_THROW(reader.seek(6), std::out_of_range);
    ASSERT_THROW(reader.seek(100), std::out_of_range);
}
