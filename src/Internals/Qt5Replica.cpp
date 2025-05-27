/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <SuitableStruct/Internals/Qt5Replica.h>

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY

#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <numeric>

namespace Qt5Replica {

namespace Internal {

//==============================================================================
// Hardware CRC32 instruction simulation (SSE4.2 / ARM CRC32)
//==============================================================================
// CRC32-C polynomial for hardware instructions (no bit inversion)
static const uint32_t CRC32C_POLYNOMIAL = 0x82F63B78;

// Generate CRC32 lookup table (without bit inversion like hardware)
static uint32_t crc32_table[256];
static bool crc32_table_initialized = false;

static void init_crc32_table() {
    if (crc32_table_initialized) return;

    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ ((crc & 1) ? CRC32C_POLYNOMIAL : 0);
        }
        crc32_table[i] = crc;
    }
    crc32_table_initialized = true;
}

// Simulate hardware CRC32 instructions (no inversion, direct computation)
static uint32_t crc32_hardware_sim(const void* data, size_t length, uint32_t crc) {
    init_crc32_table();

    const uint8_t* bytes = static_cast<const uint8_t*>(data);

    // Process 8-byte chunks (like _mm_crc32_u64)
    while (length >= 8) {
        for (int i = 0; i < 8; i++) {
            crc = (crc >> 8) ^ crc32_table[(crc ^ bytes[i]) & 0xFF];
        }
        bytes += 8;
        length -= 8;
    }

    // Process 4-byte chunk (like _mm_crc32_u32)
    if (length >= 4) {
        for (int i = 0; i < 4; i++) {
            crc = (crc >> 8) ^ crc32_table[(crc ^ bytes[i]) & 0xFF];
        }
        bytes += 4;
        length -= 4;
    }

    // Process 2-byte chunk (like _mm_crc32_u16)
    if (length >= 2) {
        for (int i = 0; i < 2; i++) {
            crc = (crc >> 8) ^ crc32_table[(crc ^ bytes[i]) & 0xFF];
        }
        bytes += 2;
        length -= 2;
    }

    // Process remaining byte (like _mm_crc32_u8)
    if (length == 1) {
        crc = (crc >> 8) ^ crc32_table[(crc ^ bytes[0]) & 0xFF];
    }

    return crc;
}

//==============================================================================
// Basic hash function (equivalent to Qt's internal hash() function)
// From qtbase/src/corelib/tools/qhash.cpp lines 199-207
//==============================================================================
static uint basicHash(const uchar *p, size_t len, uint seed) {
    uint h = seed;
    for (size_t i = 0; i < len; ++i) {
        h = 31 * h + p[i];
    }
    return h;
}

//==============================================================================
// Hash for quintptr (equivalent to Qt's qHash(quintptr))
// From qtbase/src/corelib/tools/qhashfunctions.h lines 79-83
//==============================================================================
uint customQHash(quintptr key, uint seed) {
    if (sizeof(quintptr) > sizeof(uint)) {
        return uint(((key >> (8 * sizeof(uint) - 1)) ^ key) & (~0U)) ^ seed;
    } else {
        return uint(key & (~0U)) ^ seed;
    }
}

//==============================================================================
// Hash for nullptr (equivalent to Qt's qHash(nullptr_t))
// From qtbase/src/corelib/tools/qhashfunctions.h lines 88-91
//==============================================================================
uint customQHash(std::nullptr_t, uint seed) {
    return customQHash(reinterpret_cast<quintptr>(nullptr), seed);
}

//==============================================================================
// Hash for bool (equivalent to Qt's qHash(bool))
// From qtbase/src/corelib/tools/qhashfunctions.h (bool is handled as uint)
//==============================================================================
uint customQHash(bool value, uint seed) {
    return uint(value) ^ seed;
}

//==============================================================================
// Hash for double (equivalent to Qt's qHash(double))
// From qtbase/src/corelib/tools/qhash.cpp lines 947-950
//==============================================================================
uint customQHash(double key, uint seed) {
    return key != 0.0 ? basicHash(reinterpret_cast<const uchar *>(&key), sizeof(key), seed) : seed;
}

//==============================================================================
// Hash for QString (equivalent to Qt's qHash(QString))
// From qtbase/src/corelib/tools/qhash.cpp lines 218-226 and 233
// Now with hardware CRC32 simulation for full Qt compatibility
//==============================================================================
uint customQHash(const QString &key, uint seed) {
    uint h = seed;

    // Qt uses hardware CRC32 when seed != 0 && hasFastCrc32()
    // We simulate this behavior with our hardware-compatible implementation
    if (seed != 0) {
        // Use hardware CRC32 simulation for compatibility with Qt
        const QChar* data = key.unicode();
        size_t byte_length = key.size() * sizeof(QChar);
        h = crc32_hardware_sim(data, byte_length, h);
    } else {
        // Use simple hash when seed == 0
        for (int i = 0; i < key.size(); ++i) {
            h = 31 * h + key.at(i).unicode();
        }
    }

    return h;
}

//==============================================================================
// Hash combiner structure (equivalent to QtPrivate::QHashCombine)
// From qtbase/src/corelib/tools/qhashfunctions.h lines 107-110
//==============================================================================
struct QHashCombineImpl {
    typedef uint result_type;
    template <typename T>
    result_type operator()(uint seed, const T& t) const {
        // Qt's QHashCombine calls qHash(t) WITHOUT seed, then combines
        return seed ^ (customQHash(t) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    }
};

//==============================================================================
// Hash range function (equivalent to qHashRange)
// From qtbase/src/corelib/tools/qhashfunctions.h lines 118-122
//==============================================================================
template<typename InputIterator>
uint customQHashRange(InputIterator first, InputIterator last, uint seed) {
    QHashCombineImpl combiner;
    return std::accumulate(first, last, seed, combiner);
}

// Explicit instantiations for Qt iterators
template uint customQHashRange<QJsonArray::const_iterator>(
    QJsonArray::const_iterator first,
    QJsonArray::const_iterator last,
    uint seed);

//==============================================================================
// Hash for std::pair (used internally for QJsonObject)
// From qtbase/src/corelib/tools/qhashfunctions.h lines 138-143
//==============================================================================
template<typename T1, typename T2>
uint customQHashPair(const T1& first, const T2& second, uint seed) {
    // Qt uses QtPrivate::QHashCombine for std::pair, not the QPair formula
    QHashCombineImpl hash;
    seed = hash(seed, first);
    seed = hash(seed, second);
    return seed;
}

// Explicit instantiation for QString-QJsonValue pairs
template uint customQHashPair<QString, QJsonValue>(
    const QString& first,
    const QJsonValue& second,
    uint seed);

//==============================================================================
// Hash for QJsonArray (equivalent to Qt's qHash(QJsonArray))
// From qtbase/src/corelib/serialization/qjsonarray.cpp line 1138
//==============================================================================
uint customQHash(const QJsonArray &array, uint seed) {
    return customQHashRange(array.begin(), array.end(), seed);
}

//==============================================================================
// Hash for QJsonObject (equivalent to Qt's qHash(QJsonObject))
// From qtbase/src/corelib/serialization/qjsonobject.cpp lines 1498-1506
//==============================================================================
uint customQHash(const QJsonObject &object, uint seed) {
    // Qt does: seed = hash(seed, std::pair<const QString&, const QJsonValue&>(key, value))
    // Where hash is QtPrivate::QHashCombine

    for (auto it = object.begin(), end = object.end(); it != end; ++it) {
        const QString &key = it.key();
        const QJsonValue &value = it.value();

        // Step 1: Calculate qHash(std::pair) with seed=0
        // This simulates qHash(const std::pair<T1, T2>&, uint seed=0)
        uint pairSeed = 0;

        // hash(pairSeed, key)
        uint keyHash = customQHash(key);  // qHash(key) without seed
        pairSeed = pairSeed ^ (keyHash + 0x9e3779b9 + (pairSeed << 6) + (pairSeed >> 2));

        // hash(pairSeed, value)
        uint valueHash = customQHash(value);  // qHash(value) without seed
        pairSeed = pairSeed ^ (valueHash + 0x9e3779b9 + (pairSeed << 6) + (pairSeed >> 2));

        // Step 2: Now combine the pair hash with the main seed
        // This simulates the outer QHashCombine::operator()(seed, pair)
        seed = seed ^ (pairSeed + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    }
    return seed;
}

//==============================================================================
// Hash for QJsonValue (equivalent to Qt's qHash(QJsonValue))
// From qtbase/src/corelib/serialization/qjsonvalue.cpp lines 937-948
//==============================================================================
uint customQHash(const QJsonValue &value, uint seed) {
    switch (value.type()) {
    case QJsonValue::Null:
        return customQHash(nullptr, seed);
    case QJsonValue::Bool:
        return customQHash(value.toBool(), seed);
    case QJsonValue::Double:
        return customQHash(value.toDouble(), seed);
    case QJsonValue::String:
        return customQHash(value.toString(), seed);
    case QJsonValue::Array:
        return customQHash(value.toArray(), seed);
    case QJsonValue::Object:
        return customQHash(value.toObject(), seed);
    case QJsonValue::Undefined:
        return seed;
    }
    Q_UNREACHABLE();
    return 0;
}

} // namespace Internal

} // namespace Qt5Replica

#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
