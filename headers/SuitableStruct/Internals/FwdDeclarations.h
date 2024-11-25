/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <SuitableStruct/Buffer.h>

class QJsonValue;

namespace SuitableStruct {

class BufferReader;

template<typename T> Buffer ssSave(const T& obj, bool protectedMode = true);
template<typename T> void ssLoad(BufferReader& buffer, T& obj, bool protectedMode = true);
template<typename T> [[nodiscard]] T ssLoadRet(BufferReader& reader, bool protectedMode = true);

template<typename T> QJsonValue ssJsonSave(const T& obj, bool protectedMode = true);
template<typename T> void ssJsonLoad(const QJsonValue& value, T& obj, bool protectedMode = true);
template<typename T> [[nodiscard]] T ssJsonLoadRet(const QJsonValue& value, bool protectedMode = true);

} // namespace SuitableStruct
