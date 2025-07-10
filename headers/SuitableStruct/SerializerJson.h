/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once

#ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>

#include <cstdint>
#include <type_traits>
#include <tuple>
#include <SuitableStruct/Internals/FwdDeclarations.h>
#include <SuitableStruct/Internals/DefaultTypes.h>
#include <SuitableStruct/Internals/Helpers.h>
#include <SuitableStruct/Exceptions.h>
#include <SuitableStruct/Internals/Version.h>
#include <SuitableStruct/Internals/Common.h>
#include <SuitableStruct/Handlers.h>

namespace SuitableStruct {

namespace Internal {
// JSON specific constants
extern const QString KEY_SS_FORMAT_VERSION;
extern const QString VALUE_SS_FORMAT_VERSION_1_0;
extern const QString KEY_SEGMENTS;
extern const QString KEY_VERSION_INDEX;
extern const QString KEY_DATA;

// Legacy keys (some reused for primitives)
extern const QString KEY_HASH;
extern const QString KEY_CONTENT; // Used for all types in legacy F0 format and
                                  // for primitive types in protected mode of modern F1 format.
extern const QString KEY_VERSION; // Legacy version key within old content.

uint32_t ssJsonHashValue_F0(const QJsonValue& value);
uint32_t ssJsonHashValue_F1(const QJsonValue& value);
uint32_t ssJsonHashValue(const QJsonValue& value);
} // namespace Internal

// Format detection function for JSON
[[nodiscard]] std::optional<SSDataFormat> ssDetectJsonFormat(const QJsonValue& value);

// --------- Forward declarations ---------
template<typename T> QJsonValue ssJsonSaveInternal(const T& obj);
template<typename T> void ssJsonLoadInternal(const QJsonValue& value, T& obj);

template<typename T>
void ssJsonLoadAndConvert(const QJsonValue& value, T& obj, const std::optional<uint8_t>& verOpt);

template<size_t Index, typename VersionsTuple, typename CurrentType>
void ssJsonSaveAppendSegment(QJsonArray& segmentsArray, const CurrentType& obj);
// --------- ---------

// ========= SAVE FUNCTIONS =========

template<typename T>
QJsonValue ssJsonSaveInternal(const T& obj) {
    QJsonValue part;

    if constexpr (std::is_class_v<T>) {
        // All class types produce a segments array internally.
        // SSVersions_t<T> defaults to std::tuple<T> if no ssVersions is defined,
        // so SSVersion<T>::value will be 0.
        QJsonArray segmentsArray;
        ssJsonSaveAppendSegment<SSVersion<T>::value, SSVersions_t<T>>(segmentsArray, obj);
        part = segmentsArray;
    } else {
        // Primitives are serialized directly.
        ssBeforeSaveImpl(obj);
        part = ssJsonSaveImpl(obj);
        ssAfterSaveImpl(obj);
    }

    return part;
}

// Implementation for tuple save
template<size_t I = 0, typename... Args, typename... Args2,
         typename std::enable_if<I == sizeof...(Args)>::type* = nullptr>
void ssJsonSaveImplViaTuple(QJsonObject&, const std::tuple<Args...>&, const std::tuple<Args2...>&)
{
}

template<size_t I = 0, typename... Args, typename... Args2,
         typename std::enable_if<!(I >= sizeof...(Args))>::type* = nullptr>
void ssJsonSaveImplViaTuple(QJsonObject& value, const std::tuple<Args...>& args, const std::tuple<Args2...>& names)
{
    static_assert(sizeof...(Args) == sizeof...(Args2), "Mismatching args/names size! (Check ssTuple and ssNamesTuple)");
    value[std::get<I>(names)] = ssJsonSave(std::get<I>(args), false); // Call main save with protectedMode=false
    ssJsonSaveImplViaTuple<I+1>(value, args, names);
}

// ssJsonSave.  1) Method
template<typename T,
         typename std::enable_if<can_ssJsonSaveImpl<T>::value>::type* = nullptr>
QJsonValue ssJsonSaveImpl(const T& obj)
{
    return obj.ssJsonSaveImpl();
}

// ssJsonSave.  2) Handlers
template<typename T,
         typename std::enable_if<
             !can_ssJsonSaveImpl<T>::value &&
             ::SuitableStruct::Handlers<T>::value
             >::type* = nullptr>
QJsonValue ssJsonSaveImpl(const T& obj)
{
    return ::SuitableStruct::Handlers<T>::ssJsonSaveImpl(obj);
}

// ssJsonSave.  3) Tuple
template<typename T,
         typename std::enable_if<
             !can_ssJsonSaveImpl<T>::value &&
             !::SuitableStruct::Handlers<T>::value &&
              can_ssTuple<T>::value && can_ssNamesTuple<T>::value
             >::type* = nullptr>
QJsonValue ssJsonSaveImpl(const T& obj)
{
    QJsonObject result;
    ssJsonSaveImplViaTuple(result, obj.ssTuple(), obj.ssNamesTuple());
    return result;
}

template<typename T>
QJsonValue ssJsonSave(const T& obj, bool protectedMode /*= true*/)
{
    QJsonValue coreContent = ssJsonSaveInternal(obj);

    if (protectedMode) {
        QJsonObject protectedWrapper;
        if constexpr (std::is_class_v<T>) {
            // New F1 format for ALL class types
            protectedWrapper[Internal::KEY_SS_FORMAT_VERSION] = Internal::VALUE_SS_FORMAT_VERSION_1_0;
            protectedWrapper[Internal::KEY_HASH] = ssJsonSaveImpl(Internal::ssJsonHashValue(coreContent)); // Hash of segments array
            protectedWrapper[Internal::KEY_SEGMENTS] = coreContent; // coreContent is QJsonArray of segments
        } else {
            // Legacy-like F0 wrapper for PRIMITIVES in protected mode
            protectedWrapper[Internal::KEY_HASH] = ssJsonSaveImpl(Internal::ssJsonHashValue(coreContent)); // Hash of direct data
            protectedWrapper[Internal::KEY_CONTENT] = coreContent; // coreContent is direct QJsonValue for primitive
        }
        return protectedWrapper;
    } else {
        // Non-protected mode:
        // - Class types: coreContent is QJsonArray of segments
        // - Primitive types: coreContent is the direct QJsonValue
        return coreContent;
    }
}

template<size_t Index, typename VersionsTuple, typename CurrentType>
void ssJsonSaveAppendSegment(QJsonArray& segmentsArray, const CurrentType& obj)
{
    using ThisType = std::tuple_element_t<Index, VersionsTuple>;
    static_assert(std::is_same_v<ThisType, CurrentType>, "Type mismatch in ssJsonSaveAppendSegment");

    QJsonObject segment;
    segment[Internal::KEY_VERSION_INDEX] = static_cast<int>(Index);

    ssBeforeSaveImpl(obj);
    segment[Internal::KEY_DATA] = ssJsonSaveImpl(obj); // Get raw data for this version
    ssAfterSaveImpl(obj);

    segmentsArray.append(segment); // Append current version first

    if constexpr (Index > 0) {
        using PrevType = std::tuple_element_t<Index - 1, VersionsTuple>;
        auto prevObj = construct_unique<PrevType>();

        if constexpr (HasSSDowngradeToInType<ThisType, PrevType&>::value) {
            const_cast<ThisType&>(obj).ssDowngradeTo(*prevObj);
        } else if constexpr (HasSSDowngradeToInHandlers<ThisType, PrevType&>::value) {
            Handlers<ThisType>::ssDowngradeTo(obj, *prevObj);
        } else if constexpr (std::is_convertible_v<ThisType, PrevType>) {
            *prevObj = static_cast<PrevType>(obj);
        } else {
            static_assert(HasSSDowngradeToInType<ThisType, PrevType&>::value ||
                              HasSSDowngradeToInHandlers<ThisType, PrevType&>::value ||
                              std::is_convertible_v<ThisType, PrevType>,
                          "ssDowngradeTo() or convertible missing for JSON down-conversion between versions");
        }
        ssJsonSaveAppendSegment<Index - 1, VersionsTuple>(segmentsArray, *prevObj);
    }
}

// ========= LOAD FUNCTIONS =========

// Implementation for tuple load
template<size_t I = 0, typename... Args, typename... Args2,
         typename std::enable_if<I == sizeof...(Args)>::type* = nullptr>
void ssJsonLoadImplViaTuple(const QJsonObject&, std::tuple<Args...>&, const std::tuple<Args2...>&)
{
}

template<size_t I = 0, typename... Args, typename... Args2,
         typename std::enable_if<!(I >= sizeof...(Args))>::type* = nullptr>
void ssJsonLoadImplViaTuple(const QJsonObject& value, std::tuple<Args...>& args, const std::tuple<Args2...>& names)
{
    static_assert(sizeof...(Args) == sizeof...(Args2), "Mismatching args/names size! (Check ssTuple and ssNamesTuple)");
    ssJsonLoad(value[std::get<I>(names)], std::get<I>(args), SSLoadMode::NonProtectedDefault);
    ssJsonLoadImplViaTuple<I+1>(value, args, names);
}

// ssJsonLoad.  1) Method
template<typename T,
         typename std::enable_if<can_ssJsonLoadImpl<T&, QJsonValue>::value>::type* = nullptr>
void ssJsonLoadImpl(const QJsonValue& value, T& obj)
{
    obj.ssJsonLoadImpl(value);
}

// ssJsonLoad.  2) Handlers
template<typename T,
         typename std::enable_if<
             !can_ssJsonLoadImpl<T&, QJsonValue>::value &&
             ::SuitableStruct::Handlers<T>::value
             >::type* = nullptr>
void ssJsonLoadImpl(const QJsonValue& value, T& obj)
{
    ::SuitableStruct::Handlers<T>::ssJsonLoadImpl(value, obj);
}

// ssJsonLoad.  3) Tuple
template<typename T,
         typename std::enable_if<
             !can_ssJsonLoadImpl<T&, QJsonValue>::value &&
             !::SuitableStruct::Handlers<T>::value &&
              can_ssTuple<T>::value && can_ssNamesTuple<T>::value
             >::type* = nullptr>
void ssJsonLoadImpl(const QJsonValue& value, T& obj)
{
    if(!value.isObject()) Internal::throwFormat();
    const auto jsonObject = value.toObject();
    ssJsonLoadImplViaTuple(jsonObject, const_cast_tuple(obj.ssTuple()), obj.ssNamesTuple());
}

// Conversion helpers for versioning
template<size_t I, typename TargetAppType, typename LoadedSegmentType,
         typename std::enable_if<!(I <= SSVersion<TargetAppType>::value)>::type* = nullptr> // I is out of bounds for iteration
void ssJsonLoadAndConvertIter2(TargetAppType&, const LoadedSegmentType&)
{
    Internal::throwVersionError();
}

template<size_t I, typename TargetAppType, typename LoadedSegmentType,
         typename std::enable_if<I == SSVersion<TargetAppType>::value>::type* = nullptr> // I is current version of TargetAppType
void ssJsonLoadAndConvertIter2(TargetAppType& obj, const LoadedSegmentType& srcObj)
{ // srcObj is of type std::tuple_element_t<I-1, SSVersions_t<TargetAppType>> or similar if I > 0
    // We are converting from version I-1 (srcObj type) to version I (obj type, which is TargetAppType)
    if constexpr (std::is_same_v<TargetAppType, LoadedSegmentType>) {
        obj = srcObj; // Should not happen if I == SSVersion and I > 0, unless srcObj was already TargetAppType
    } else if constexpr (HasSSUpgradeFromInType<TargetAppType, const LoadedSegmentType&>::value) {
        obj.ssUpgradeFrom(srcObj);
    } else if constexpr (HasSSUpgradeFromInHandlers<TargetAppType, const LoadedSegmentType&>::value) {
        Handlers<TargetAppType>::ssUpgradeFrom(srcObj, obj);
    } else if constexpr (std::is_convertible_v<LoadedSegmentType, TargetAppType>) {
        obj = static_cast<TargetAppType>(srcObj);
    } else {
        static_assert(HasSSUpgradeFromInType<TargetAppType, const LoadedSegmentType&>::value ||
                          HasSSUpgradeFromInHandlers<TargetAppType, const LoadedSegmentType&>::value ||
                          std::is_convertible_v<LoadedSegmentType, TargetAppType>,
                      "ssUpgradeFrom() or convertible missing for final JSON up-conversion step to TargetAppType");
    }
}

// Iterate I from (VersionOfLoadedSegmentData + 1) up to SSVersion<TargetAppType>::value
// At each step, convert from std::tuple_element_t<I-1, Versions> to std::tuple_element_t<I, Versions>
template<size_t I, typename TargetAppType, typename PreviousVersionType, /* PreviousVersionType is type of version I-1 */
         typename std::enable_if< (I < SSVersion<TargetAppType>::value) && (I < std::tuple_size_v<SSVersions_t<TargetAppType>>) >::type* = nullptr>
void ssJsonLoadAndConvertIter2(TargetAppType& finalObj, const PreviousVersionType& prevVersionObj)
{
    using CurrentIterTargetType = std::tuple_element_t<I, SSVersions_t<TargetAppType>>; // Type of version I
    auto currentIterObj = construct_unique<CurrentIterTargetType>();

    if constexpr (HasSSUpgradeFromInType<CurrentIterTargetType, const PreviousVersionType&>::value) {
        currentIterObj->ssUpgradeFrom(prevVersionObj);
    } else if constexpr (HasSSUpgradeFromInHandlers<CurrentIterTargetType, const PreviousVersionType&>::value) {
        Handlers<CurrentIterTargetType>::ssUpgradeFrom(prevVersionObj, *currentIterObj);
    } else if constexpr (std::is_convertible_v<PreviousVersionType, CurrentIterTargetType>) {
        *currentIterObj = static_cast<CurrentIterTargetType>(prevVersionObj);
    } else {
        static_assert(HasSSUpgradeFromInType<CurrentIterTargetType, const PreviousVersionType&>::value ||
                          HasSSUpgradeFromInHandlers<CurrentIterTargetType, const PreviousVersionType&>::value ||
                          std::is_convertible_v<PreviousVersionType, CurrentIterTargetType>,
                      "ssUpgradeFrom() or convertible missing for intermediate JSON up-conversion step");
    }
    ssJsonLoadAndConvertIter2<I+1>(finalObj, *currentIterObj); // Convert from current (I) to next (I+1)
}

template<size_t I, typename TargetAppType,
         typename std::enable_if< !(I < std::tuple_size_v<SSVersions_t<TargetAppType>>) >::type* = nullptr>
void ssJsonLoadAndConvertIter(const QJsonValue&, TargetAppType&, uint8_t)
{
    Internal::throwVersionError(); // serializedVer out of bounds
}

// Entry point for recursive version conversion, called from ssJsonLoadAndConvert.
// Iterates I from 0 up to `serializedVer` to find the type of `serializedVer` data.
// Then calls Iter2 to convert from `serializedVer` up to `TargetAppType`.
template<size_t I, typename TargetAppType, /* TargetAppType is the type of `obj` we want to load into */
         typename std::enable_if< I < std::tuple_size_v<SSVersions_t<TargetAppType>> >::type* = nullptr>
void ssJsonLoadAndConvertIter(const QJsonValue& rawDataForSerializedVer, TargetAppType& objToPopulate, uint8_t serializedVer_actualIndex)
{
    if (serializedVer_actualIndex >= std::tuple_size_v<SSVersions_t<TargetAppType>>) {
        Internal::throwVersionError();
    }

    if (I < serializedVer_actualIndex) {
        ssJsonLoadAndConvertIter<I+1>(rawDataForSerializedVer, objToPopulate, serializedVer_actualIndex);
    } else if (I == serializedVer_actualIndex) {
        // I is now the index of the type that rawDataForSerializedVer represents.
        using TypeOfSerializedData = std::tuple_element_t<I, SSVersions_t<TargetAppType>>;
        auto loadedSerializedVerObject = construct_unique<TypeOfSerializedData>();
        ssJsonLoadImpl(rawDataForSerializedVer, *loadedSerializedVerObject); // Load raw data into its original type

        if constexpr (I == SSVersion<TargetAppType>::value) { // Is serializedVer also the current app version?
            objToPopulate = std::move(*loadedSerializedVerObject); // Yes, exact match, just move.
        } else {
            // No, serializedVer is older. Start iterative conversion from its type up to TargetAppType.
            // Start Iter2 from I + 1.
            ssJsonLoadAndConvertIter2<I + 1>(objToPopulate, *loadedSerializedVerObject);
        }
    } else {
        Internal::throwVersionError();
    }
}

template<typename T>
void ssJsonLoadAndConvert(const QJsonValue& value, T& obj, const std::optional<uint8_t>& verOpt)
{
    // 'value' is the actual content data (e.g., from a segment's "data" field or legacy "content")
    // 'verOpt' is the version_index of this 'value' as read from the stream.
    if constexpr (HasSSVersionsInTypeOrHandlers<T>::value) {
        if (verOpt) {
            const uint8_t serializedVerIdx = *verOpt;
            if (serializedVerIdx >= std::tuple_size_v<SSVersions_t<T>>) {
                Internal::throwVersionError(); // Serialized version index is out of bounds for type T's defined versions
            }
            // Start iterating from version 0 up to serializedVerIdx to load and then convert
            ssJsonLoadAndConvertIter<0>(value, obj, serializedVerIdx);
        } else {
            // No version_index provided with the data, but T is versioned.
            // This implies the data should be for the current version of T, or it's an error.
            // Let's assume it should be current version if no explicit index.
            if constexpr (SSVersion<T>::value == 0) { // T has only one version (itself)
                ssJsonLoadImpl(value, obj); // Load directly as current version
            } else {
                // T has multiple versions, but no ver_index given for this data. This is ambiguous/error.
                Internal::throwFormat();
            }
        }
    } else { // Type T is not versioned (e.g. primitive or class not in ssVersions system)
        if (verOpt.has_value() && verOpt.value() != 0) {
            Internal::throwVersionError(); // Data claims to be version X, but T is not versioned like that.
        }
        ssJsonLoadImpl(value, obj); // Load directly
    }
}

template<typename T>
[[nodiscard]] T ssJsonLoadImplRet(const QJsonValue& value)
{
    auto result = construct<T>();
    ssJsonLoadImpl(value, result);
    return result;
}

template<typename T>
void ssJsonLoad(const QJsonValue& value, T& obj, SSLoadMode loadMode /*= SSLoadMode::Protected*/)
{
    auto temp = construct_unique<T>();

    if (loadMode == SSLoadMode::Protected) {
        if (!value.isObject()) Internal::throwFormat();
        const auto rootObject = value.toObject();

        if (rootObject.contains(Internal::KEY_SS_FORMAT_VERSION)) { // New F1 format (expected for all classes)
            if (rootObject[Internal::KEY_SS_FORMAT_VERSION].toString() != Internal::VALUE_SS_FORMAT_VERSION_1_0) {
                Internal::throwFormat();
            }
            if (!rootObject.contains(Internal::KEY_HASH) || !rootObject.contains(Internal::KEY_SEGMENTS)) {
                Internal::throwFormat();
            }
            if constexpr (!std::is_class_v<T>) {
                 Internal::throwFormat();
            }

            Internal::LegacyFormatScope legacyScope(Internal::FormatType::Json, false);
            const QJsonValue segmentsValue = rootObject[Internal::KEY_SEGMENTS];
            uint32_t expectedHash;
            ssJsonLoadImpl(rootObject[Internal::KEY_HASH], expectedHash); // Hash is a uint saved as JSON

            if (expectedHash != Internal::ssJsonHashValue(segmentsValue)) {
                Internal::throwIntegrity();
            }
            ssJsonLoadInternal(segmentsValue, *temp); // This calls before/after hooks internally

        } else if (rootObject.contains(Internal::KEY_HASH) && rootObject.contains(Internal::KEY_CONTENT)) { // Legacy F0 format
            Internal::LegacyFormatScope legacyScope(Internal::FormatType::Json, true);
            const QJsonValue legacyContent = rootObject[Internal::KEY_CONTENT];
            uint32_t expectedHash;
            ssJsonLoadImpl(rootObject[Internal::KEY_HASH], expectedHash);

            if (expectedHash != Internal::ssJsonHashValue_F0(legacyContent)) {
                Internal::throwIntegrity();
            }

            // Legacy format: let ssJsonLoadInternal handle the version wrapper for classes or direct data for primitives
            ssJsonLoadInternal(legacyContent, *temp);

        } else {
            Internal::throwFormat();
        }

    } else { // Non-protected mode
        // Determine format based on hints or auto-detection
        bool useF1Format = [loadMode](){
            switch (loadMode) {
                case SSLoadMode::NonProtectedF0Hint:  return false;
                case SSLoadMode::NonProtectedF1Hint:  return true;
                case SSLoadMode::NonProtectedDefault: return !isProcessingLegacyFormatOpt(Internal::FormatType::Json).value_or(false);
                case SSLoadMode::Protected:
                    assert(false && "Should never reach here");
                    return false;
            }

            assert(false && "Should never reach here");
            return false;
        }();

        Internal::LegacyFormatScope legacyScope(Internal::FormatType::Json, !useF1Format);
        ssJsonLoadInternal(value, *temp);
    }
    obj = std::move(*temp);
}

template<typename T>
[[nodiscard]] T ssJsonLoadRet(const QJsonValue& value, SSLoadMode loadMode /*= SSLoadMode::Protected*/)
{
    auto result = construct<T>();
    ssJsonLoad(value, result, loadMode);
    return result;
}

template<typename T>
void ssJsonLoadInternal(const QJsonValue& value, T& obj) {
    ssBeforeLoadImpl(obj);

    if constexpr (std::is_class_v<T>) {
        if (Internal::isProcessingLegacyFormat(Internal::FormatType::Json)) { // Legacy format expected
            // `value` is expected to be a QJsonObject like {"version":X, "content":Y} or just {Y} if no version
            QJsonValue actualData = value;
            std::optional<uint8_t> legacyVerOpt;

            if (value.isObject()) {
                const QJsonObject valObj = value.toObject();
                if (valObj.contains(Internal::KEY_VERSION) && valObj.contains(Internal::KEY_CONTENT)) {
                    legacyVerOpt = static_cast<uint8_t>(valObj[Internal::KEY_VERSION].toInt());
                    actualData = valObj[Internal::KEY_CONTENT];
                } else if (valObj.contains(Internal::KEY_VERSION)) {
                    legacyVerOpt = static_cast<uint8_t>(valObj[Internal::KEY_VERSION].toInt());
                    // actualData remains valObj, ssJsonLoadAndConvert will decide
                } else {
                    // No version/content keys, actualData is the value itself (might be an object representing the class)
                }
            }
            ssJsonLoadAndConvert(actualData, obj, legacyVerOpt);

        } else { // Modern F1 format expected (segments array)
            if (!value.isArray()) {
                Internal::throwFormat(); // Expect segments array for classes in modern format
            }
            const QJsonArray segmentsArray = value.toArray();
            const auto desiredVersionIdx = static_cast<int>(SSVersion<T>::value);
            bool loaded = false;

            // First pass: look for exact version match
            for (const QJsonValue& segmentVal : segmentsArray) {
                if (!segmentVal.isObject()) { Internal::throwFormat(); }
                const QJsonObject segmentObj = segmentVal.toObject();

                if (!segmentObj.contains(Internal::KEY_VERSION_INDEX) || !segmentObj.contains(Internal::KEY_DATA)) {
                    Internal::throwFormat();
                }

                const int storedVersionIdx = segmentObj[Internal::KEY_VERSION_INDEX].toInt();
                const QJsonValue versionData = segmentObj[Internal::KEY_DATA];

                if (storedVersionIdx == desiredVersionIdx) {
                    ssJsonLoadImpl(versionData, obj); // Direct load of the exact version
                    loaded = true;
                    break;
                }
            }

            // Second pass: if no exact match, look for upgradeable versions
            if (!loaded) {
                for (const QJsonValue& segmentVal : segmentsArray) {
                    if (!segmentVal.isObject()) { Internal::throwFormat(); }
                    const QJsonObject segmentObj = segmentVal.toObject();

                    if (!segmentObj.contains(Internal::KEY_VERSION_INDEX) || !segmentObj.contains(Internal::KEY_DATA)) {
                        Internal::throwFormat();
                    }

                    const int storedVersionIdx = segmentObj[Internal::KEY_VERSION_INDEX].toInt();
                    const QJsonValue versionData = segmentObj[Internal::KEY_DATA];

                    if (storedVersionIdx < desiredVersionIdx && storedVersionIdx >= 0) {
                        // Load the older version and let ssJsonLoadAndConvert handle upgrades.
                        ssJsonLoadAndConvert(versionData, obj, static_cast<uint8_t>(storedVersionIdx));
                        loaded = true;
                        break;
                    }
                    // If storedVersionIdx > desiredVersionIdx, skip (older client, newer data segment).
                }
            }

            if (!loaded) {
                Internal::throwVersionError();
            }
        }

    } else { // Primitives
        // For primitives, value is the direct data.
        ssJsonLoadImpl(value, obj);
    }

    ssAfterLoadImpl(obj);
}

} // namespace SuitableStruct

#endif // #ifdef SUITABLE_STRUCT_HAS_QT_LIBRARY
