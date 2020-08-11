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
    const auto strHelloSz = sizeof(strHello) - 1;
    const auto strWorldSz = sizeof(strWorld) - 1;

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
