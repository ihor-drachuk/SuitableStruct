/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <SuitableStruct/Buffer.h>

class QJsonValue;

namespace SuitableStruct {

template<typename T> Buffer ssSave(const T& obj, bool protectedMode = true);
template<typename T> QJsonValue ssJsonSave(const T& obj, bool protectedMode = true);
template<typename T> void ssJsonLoad(const QJsonValue& value, T& obj, bool protectedMode = true);

} // namespace SuitableStruct
