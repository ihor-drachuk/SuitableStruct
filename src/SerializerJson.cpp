/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY

#include <SuitableStruct/SerializerJson.h>
#include <SuitableStruct/Internals/Qt5Replica.h>
#include <SuitableStruct/Buffer.h>
#include <QJsonDocument>

namespace SuitableStruct {
namespace Internal {

// JSON specific constants
const QString KEY_SS_FORMAT_VERSION = QStringLiteral("ss_format_version");
const QString VALUE_SS_FORMAT_VERSION_1_0 = QStringLiteral("1.0");
const QString KEY_SEGMENTS = QStringLiteral("segments");
const QString KEY_VERSION_INDEX = QStringLiteral("version_index");
const QString KEY_DATA = QStringLiteral("data");

// Legacy keys (some reused for primitives)
const QString KEY_HASH = QStringLiteral("hash");
const QString KEY_CONTENT = QStringLiteral("content"); // Used for all types in legacy F0 format
                                                       // and for primitive types in protected mode of modern F1 format.
const QString KEY_VERSION = QStringLiteral("version"); // Legacy version key within old content.

static Buffer jsonValueToBuffer(const QJsonValue& value)
{
    QJsonDocument doc;

    if (value.isObject()) {
        doc = QJsonDocument(value.toObject());
    } else if (value.isArray()) {
        doc = QJsonDocument(value.toArray());
    } else {
        QJsonArray wrapper;
        wrapper.append(value);
        doc = QJsonDocument(wrapper);
    }

    return Buffer::fromQByteArray(doc.toJson(QJsonDocument::Compact));
}

// Custom JSON hash function that's incompatible with old implementations
uint32_t ssJsonHashValue(const QJsonValue& value)
{
    return ssJsonHashValue_F1(value);
}

// Legacy JSON hash function for backward compatibility (equivalent to qHash)
uint32_t ssJsonHashValue_F0(const QJsonValue& value)
{
    return static_cast<uint32_t>(Qt5Replica::qHash(value));
}

// Custom JSON hash function that's incompatible with old implementations
uint32_t ssJsonHashValue_F1(const QJsonValue& value)
{
    return jsonValueToBuffer(value).hash();
}

} // namespace Internal

std::optional<SSDataFormat> ssDetectJsonFormat(const QJsonValue& value)
{
    try {
        // Check if this is a protected mode JSON (should be an object)
        if (!value.isObject())
            return {};

        const QJsonObject rootObject = value.toObject();

        // Check for new F1 format markers
        if (rootObject.contains(Internal::KEY_SS_FORMAT_VERSION)) {
            if (rootObject[Internal::KEY_SS_FORMAT_VERSION].toString() == Internal::VALUE_SS_FORMAT_VERSION_1_0) {
                // This looks like F1 format
                if (rootObject.contains(Internal::KEY_HASH) && rootObject.contains(Internal::KEY_SEGMENTS))
                    return SSDataFormat::F1;
            }
            return {}; // Invalid F1 format
        }

        // Check for legacy F0 format markers
        if (rootObject.contains(Internal::KEY_HASH) && rootObject.contains(Internal::KEY_CONTENT)) {
            // This looks like F0 format - let's try to validate the hash
            const QJsonValue legacyContent = rootObject[Internal::KEY_CONTENT];
            const QJsonValue hashValue = rootObject[Internal::KEY_HASH];

            if (hashValue.isDouble() || hashValue.isString()) {
                // Try to extract hash and validate
                uint32_t expectedHash = 0;
                if (hashValue.isDouble()) {
                    expectedHash = static_cast<uint32_t>(hashValue.toDouble());
                } else {
                    bool ok = false;
                    expectedHash = hashValue.toString().toUInt(&ok);
                    if (!ok)
                        return {};
                }

                const uint32_t computedHash = Internal::ssJsonHashValue_F0(legacyContent);
                if (expectedHash == computedHash)
                    return SSDataFormat::F0;
            }

            return {}; // Invalid hash
        }

        // Doesn't match any known protected format
        return {};

    } catch (...) {
        return {};
    }
}

} // namespace SuitableStruct

#endif // SUITABLE_STRUCT_HAS_QT_LIBRARY
