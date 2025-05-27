/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY

#include <QtGlobal>

class QJsonValue;
class QJsonObject;
class QJsonArray;
class QString;

namespace Qt5Replica {

namespace Internal {

// Custom hash functions that replicate Qt 5.15.2's qHash behavior exactly.
// These functions produce identical results to Qt5's built-in qHash functions.

// Main hash function for QJsonValue
uint customQHash(const QJsonValue& value, uint seed = 0);

// Hash functions for JSON container types
uint customQHash(const QJsonArray& array, uint seed = 0);
uint customQHash(const QJsonObject& object, uint seed = 0);

// Hash functions for basic Qt types
uint customQHash(bool value, uint seed = 0);
uint customQHash(double value, uint seed = 0);
uint customQHash(const QString& value, uint seed = 0);
uint customQHash(quintptr value, uint seed = 0);
uint customQHash(std::nullptr_t, uint seed = 0);

// Hash range function (equivalent to qHashRange)
template<typename InputIterator>
uint customQHashRange(InputIterator first, InputIterator last, uint seed = 0);

// Hash for pairs (used internally for QJsonObject)
template<typename T1, typename T2>
uint customQHashPair(const T1& first, const T2& second, uint seed = 0);

} // namespace Internal

template<typename T>
uint qHash(const T& value, uint seed = 0)
{
    return Internal::customQHash(value, seed);
}

} // namespace Qt5Replica

#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
