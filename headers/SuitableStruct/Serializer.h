/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <tuple>
#include <limits>
#include <memory>
#include <optional>
#include <SuitableStruct/Internals/FwdDeclarations.h>
#include <SuitableStruct/Internals/Helpers.h>
#include <SuitableStruct/Internals/Version.h>
#include <SuitableStruct/Internals/DefaultTypes.h>
#include <SuitableStruct/Exceptions.h>
#include <SuitableStruct/Buffer.h>
#include <SuitableStruct/BufferReader.h>
#include <SuitableStruct/Handlers.h>
#include <SuitableStruct/Hashes.h>

// Features:
//   - Serialization / deserialization for structures
//   - Data integrity validation
//   - Versioning

// TODO:
//  - SSO-replacement for std::optional

namespace SuitableStruct {

namespace Internal {

// Format constants
constexpr size_t SS_FORMAT_MARK_SIZE = 5;
extern const uint8_t SS_FORMAT_F0[SS_FORMAT_MARK_SIZE];  // Format F0, single-version, old hash algorithm
extern const uint8_t SS_FORMAT_F1[SS_FORMAT_MARK_SIZE];  // Format F1, multiple versions segments, new hash algorithm

} // namespace Internal

// ------ Forward declarations ------
template<typename T> Buffer ssSaveInternal(const T& obj);
template<typename T> void ssLoadInternal(BufferReader& bufferReader, T& obj);
template<typename T> [[nodiscard]] T ssLoadInternalRet(BufferReader& bufferReader);

template<typename T, typename std::enable_if<can_ssSaveImpl<T>::value>::type* = nullptr> Buffer ssSaveImpl(const T& obj);
template<typename T, typename std::enable_if<!can_ssSaveImpl<T>::value && Handlers<T>::value>::type* = nullptr> Buffer ssSaveImpl(const T& obj);
template<typename T, typename std::enable_if<!can_ssSaveImpl<T>::value && !Handlers<T>::value && can_ssTuple<T>::value>::type* = nullptr> Buffer ssSaveImpl(const T& obj);

template<typename T>
void ssLoadAndConvert(BufferReader& bufferReader, T& obj, const std::optional<uint8_t>& ver);
// ------ ------

template<size_t I = 0, typename... Args,
         typename std::enable_if<I == sizeof...(Args)>::type* = nullptr>
void ssSaveImplViaTuple(Buffer&, const std::tuple<Args...>&)
{
}

template<size_t I = 0, typename... Args,
         typename std::enable_if<!(I >= sizeof...(Args))>::type* = nullptr>
void ssSaveImplViaTuple(Buffer& buffer, const std::tuple<Args...>& args)
{
    buffer += ssSaveInternal(std::get<I>(args));
    ssSaveImplViaTuple<I+1>(buffer, args);
}

// Internal tuple save (F1 format - no format markers in recursive calls)
template<size_t I = 0, typename... Args,
         typename std::enable_if<I == sizeof...(Args)>::type* = nullptr>
void ssSaveImplViaTupleInternal(Buffer&, const std::tuple<Args...>&)
{
}

template<size_t I = 0, typename... Args,
         typename std::enable_if<!(I >= sizeof...(Args))>::type* = nullptr>
void ssSaveImplViaTupleInternal(Buffer& buffer, const std::tuple<Args...>& args)
{
    buffer += ssSaveInternal(std::get<I>(args));
    ssSaveImplViaTupleInternal<I+1>(buffer, args);
}

// Implementation for tuple load
template<size_t I = 0, typename... Args,
         typename std::enable_if<I == sizeof...(Args)>::type* = nullptr>
void ssLoadImplViaTuple(BufferReader&, std::tuple<Args...>&)
{
}

template<size_t I = 0, typename... Args,
         typename std::enable_if<!(I >= sizeof...(Args))>::type* = nullptr>
void ssLoadImplViaTuple(BufferReader& bufferReader, std::tuple<Args...>& args)
{
    ssLoad(bufferReader, std::get<I>(args), false);
    ssLoadImplViaTuple<I+1>(bufferReader, args);
}

// Internal tuple load (F1 format - no format markers in recursive calls)
template<size_t I = 0, typename... Args,
         typename std::enable_if<I == sizeof...(Args)>::type* = nullptr>
void ssLoadImplViaTupleInternal(BufferReader&, std::tuple<Args...>&)
{
}

template<size_t I = 0, typename... Args,
         typename std::enable_if<!(I >= sizeof...(Args))>::type* = nullptr>
void ssLoadImplViaTupleInternal(BufferReader& bufferReader, std::tuple<Args...>& args)
{
    if (Internal::isProcessingLegacyFormat(Internal::FormatType::Binary)) {
        // For F0 format, only class types have version bytes, primitives don't
        using ArgType = std::decay_t<std::tuple_element_t<I, std::tuple<Args...>>>;

        if constexpr (std::is_class_v<ArgType>) {
            // For class types, read version and use conversion logic
            uint8_t version {};
            bufferReader.read(version);
            ssLoadAndConvert(bufferReader, std::get<I>(args), version);
        } else {
            // For primitive types, no version byte - use ssLoadImpl directly
            ssLoadImpl(bufferReader, std::get<I>(args));
        }
    } else {
        // For F1 format, use ssLoadInternal (handles segments)
        ssLoadInternal(bufferReader, std::get<I>(args));
    }
    ssLoadImplViaTupleInternal<I+1>(bufferReader, args);
}

// Version conversion helpers
template<size_t I, typename T, typename T2>
void ssLoadAndConvertIter2(T& obj, T2&& srcObj)
{
    static_assert(I <= SSVersion<T>::value, "ssLoadAndConvertIter2: I is out of bounds");

    using TargetType = std::tuple_element_t<I, SSVersions_t<T>>;
    std::unique_ptr<TargetType> tempObj;
    auto& target = [&]() -> auto& {
        if constexpr (I == SSVersion<T>::value) {
            return obj;
        } else {
            tempObj = construct_unique<TargetType>();
            return *tempObj;
        }
    }();

    if constexpr (std::is_same_v<T2, TargetType>) {
        target = std::forward<T2>(srcObj);
    } else if constexpr (HasSSUpgradeFromInType<TargetType, T2&&>::value) {
        target.ssUpgradeFrom(std::forward<T2&&>(srcObj));
    } else if constexpr (HasSSUpgradeFromInHandlers<TargetType, T2&&>::value) {
        Handlers<TargetType>::ssUpgradeFrom(std::forward<T2&&>(srcObj), target);
    } else if constexpr (std::is_convertible_v<T2, TargetType>) {
        target = static_cast<TargetType>(srcObj);
    } else {
        static_assert(HasSSUpgradeFromInType<TargetType, T2&&>::value || HasSSUpgradeFromInHandlers<TargetType, T2&&>::value,
                      "ssUpgradeFrom() missing for up-conversion between versions");
    }

    if constexpr (I < SSVersion<T>::value)
        ssLoadAndConvertIter2<I+1>(obj, std::move(target));
}

template<size_t I, typename T>
void ssLoadAndConvertIter(BufferReader& bufferReader, T& obj, uint8_t serializedVer)
{
    if (serializedVer >= std::tuple_size_v<SSVersions_t<T>>) {
        // New format guarantees that serializedVer is always less than the size of SSVersions_t<T>,
        // but downgrade attempt on previous version leads us to this point.
        Internal::throwVersionError();
    }

    using CurrentType = std::tuple_element_t<I, SSVersions_t<T>>;

    if constexpr (I < std::tuple_size_v<SSVersions_t<T>> - 1) {
        static_assert(I != SSVersion<T>::value);

        if (I < serializedVer) {
            ssLoadAndConvertIter<I+1>(bufferReader, obj, serializedVer);
        } else {
            auto tempObj = construct<CurrentType>();
            ssLoadImplInternal(bufferReader, tempObj);
            ssLoadAndConvertIter2<I>(obj, std::move(tempObj));
        }

    } else if constexpr (I == std::tuple_size_v<SSVersions_t<T>> - 1) {
        assert(serializedVer == I);
        ssLoadImplInternal(bufferReader, obj);

    } else {
        //static_assert(false, "ssLoadAndConvertIter: I is out of bounds");
        assert(false && "ssLoadAndConvertIter: I is out of bounds");
    }
}

template<typename T>
void ssLoadAndConvert(BufferReader& bufferReader, T& obj, const std::optional<uint8_t>& ver)
{
    static_assert(std::is_class_v<T>);
    ssLoadAndConvertIter<0>(bufferReader, obj, ver.value_or(0));
}

template<size_t Index, typename VersionsTuple, typename CurrentType>
void ssSaveAppendSegment(Buffer& part, const CurrentType& obj)
{
    using ThisType = std::tuple_element_t<Index, VersionsTuple>;
    static_assert(std::is_same_v<ThisType, CurrentType>, "Type mismatch in ssSaveAppendSegment");

    Buffer versionData;
    ssBeforeSaveImpl(obj);
    versionData += ssSaveImpl(obj);
    ssAfterSaveImpl(obj);

    part.write(static_cast<uint8_t>(Index));
    part.write(static_cast<uint64_t>(versionData.size()));
    part += versionData;

    // Prepare previous version if exists
    if constexpr (Index > 0) {
        using PrevType = std::tuple_element_t<Index - 1, VersionsTuple>;
        PrevType prevObj = construct<PrevType>();

        if constexpr (HasSSDowngradeToInType<ThisType, PrevType&>::value) {
            const_cast<ThisType&>(obj).ssDowngradeTo(prevObj);
        } else if constexpr (HasSSDowngradeToInHandlers<ThisType, PrevType&>::value) {
            Handlers<ThisType>::ssDowngradeTo(obj, prevObj);
        } else if constexpr (std::is_convertible_v<ThisType, PrevType>) {
            prevObj = static_cast<PrevType>(obj);
        } else {
            static_assert(HasSSDowngradeToInType<ThisType, PrevType&>::value || HasSSDowngradeToInHandlers<ThisType, PrevType&>::value,
                          "ssDowngradeTo() missing for down-conversion between versions");
        }

        // Recursive call for next segment
        ssSaveAppendSegment<Index - 1, VersionsTuple>(part, prevObj);
    }
}

template<typename T>
[[nodiscard]] T ssLoadImplRet(BufferReader& bufferReader)
{
    auto result = construct<T>();
    ssLoadImpl(bufferReader, result);
    return result;
}

template<typename T>
[[nodiscard]] T ssLoadImplRet(BufferReader&& bufferReader)
{
    return ssLoadImplRet<T>(static_cast<BufferReader&>(bufferReader));
}

template<typename T>
[[nodiscard]] T ssLoadInternalRet(BufferReader& bufferReader)
{
    auto result = construct<T>();
    if (Internal::isProcessingLegacyFormat(Internal::FormatType::Binary)) {
        // For F0 format, individual objects (like variant alternatives) have version bytes
        if constexpr (std::is_class_v<T>) {
            // Class types: read version and use conversion logic
            uint8_t version {};
            bufferReader.read(version);
            ssLoadAndConvert(bufferReader, result, version);
        } else {
            // Primitive types
            ssLoadImpl(bufferReader, result);
        }
    } else {
        // For F1 format, use ssLoadInternal (handles segments)
        ssLoadInternal(bufferReader, result);
    }
    return result;
}

template<typename T>
[[nodiscard]] T ssLoadInternalRet(BufferReader&& bufferReader)
{
    return ssLoadInternalRet<T>(static_cast<BufferReader&>(bufferReader));
}

// Internal serialization functions (no format marker)
template<typename T,
         typename std::enable_if<can_ssSaveImpl<T>::value>::type* = nullptr>
Buffer ssSaveImplInternal(const T& obj)
{
    return obj.ssSaveImpl();
}

template<typename T,
         typename std::enable_if<
             !can_ssSaveImpl<T>::value &&
             Handlers<T>::value
             >::type* = nullptr>
Buffer ssSaveImplInternal(const T& obj)
{
    return Handlers<T>::ssSaveImpl(obj);
}

template<typename T,
         typename std::enable_if<
             !can_ssSaveImpl<T>::value &&
             !Handlers<T>::value &&
              can_ssTuple<T>::value
             >::type* = nullptr>
Buffer ssSaveImplInternal(const T& obj)
{
    Buffer buf;
    ssSaveImplViaTupleInternal(buf, obj.ssTuple());
    return buf;
}

// ssSave.  1) Method
template<typename T,
         typename std::enable_if<can_ssSaveImpl<T>::value>::type* /*= nullptr*/>
Buffer ssSaveImpl(const T& obj)
{
    return obj.ssSaveImpl();
}

// ssSave.  2) Handlers
template<typename T,
         typename std::enable_if<
             !can_ssSaveImpl<T>::value &&
             Handlers<T>::value
             >::type* /*= nullptr*/>
Buffer ssSaveImpl(const T& obj)
{
    return Handlers<T>::ssSaveImpl(obj);
}

// ssSave.  3) Tuple
template<typename T,
         typename std::enable_if<
             !can_ssSaveImpl<T>::value &&
             !Handlers<T>::value &&
              can_ssTuple<T>::value
             >::type* /*= nullptr*/>
Buffer ssSaveImpl(const T& obj)
{
    Buffer buf;
    ssSaveImplViaTuple(buf, obj.ssTuple());
    return buf;
}

template<typename T>
Buffer ssSaveInternal(const T& obj)
{
    Buffer part;

    if constexpr (std::is_class_v<T>) {
        const uint8_t segmentsCount = static_cast<uint8_t>(std::tuple_size_v<SSVersions_t<T>>);
        part.write(segmentsCount);
        ssSaveAppendSegment<SSVersion<T>::value, SSVersions_t<T>>(part, obj);
    } else {
        // Primitive types
        ssBeforeSaveImpl(obj);
        part += ssSaveImpl(obj);
        ssAfterSaveImpl(obj);
    }

    return part;
}

template<typename T>
Buffer ssSave(const T& obj, bool protectedMode /*= true*/)
{
    Buffer part;

    // Only write format marker for protected mode (root level)
    if (protectedMode) {
        part.writeRaw(static_cast<const void*>(Internal::SS_FORMAT_F1), sizeof(Internal::SS_FORMAT_F1)); // Format mark
    }

    // Use internal save logic
    part += ssSaveInternal(obj);

    if (protectedMode) {
        Buffer result;
        static_assert (sizeof(part.hash()) == sizeof(uint32_t), "Make sure save & load expect same type!");
        result.write(static_cast<uint64_t>(part.size()));
        result.write(part.hash());
        result += part;
        return result;
    } else {
        return part;
    }
}

// Internal load functions (no format reading)
template<typename T,
         typename std::enable_if<can_ssLoadImpl<T&, BufferReader&>::value>::type* = nullptr>
void ssLoadImplInternal(BufferReader& bufferReader, T& obj)
{
    obj.ssLoadImpl(bufferReader);
}

template<typename T,
         typename std::enable_if<
             !can_ssLoadImpl<T&, BufferReader&>::value &&
             Handlers<T>::value
             >::type* = nullptr>
void ssLoadImplInternal(BufferReader& bufferReader, T& obj)
{
    Handlers<T>::ssLoadImpl(bufferReader, obj);
}

template<typename T,
         typename std::enable_if<
             !can_ssLoadImpl<T&, BufferReader&>::value &&
             !Handlers<T>::value &&
              can_ssTuple<T>::value
             >::type* = nullptr>
void ssLoadImplInternal(BufferReader& bufferReader, T& obj)
{
    ssLoadImplViaTupleInternal(bufferReader, const_cast_tuple(obj.ssTuple()));
}

// Fallback for ssLoadImplInternal: when T does not have ssLoadImpl, no Handlers, and no ssTuple
template<typename T,
         typename std::enable_if<
             !can_ssLoadImpl<T&, BufferReader&>::value &&
             !Handlers<T>::value &&
             !can_ssTuple<T>::value
         >::type* = nullptr>
void ssLoadImplInternal(BufferReader& bufferReader, T& obj)
{
    ssLoadImpl(bufferReader, obj);
}

// ssLoad.  1) Method
template<typename T,
         typename std::enable_if<can_ssLoadImpl<T&, BufferReader&>::value>::type* = nullptr>
void ssLoadImpl(BufferReader& bufferReader, T& obj)
{
    obj.ssLoadImpl(bufferReader);
}

// ssLoad.  2) Handlers
template<typename T,
         typename std::enable_if<
             !can_ssLoadImpl<T&, BufferReader&>::value &&
             Handlers<T>::value
             >::type* = nullptr>
void ssLoadImpl(BufferReader& bufferReader, T& obj)
{
    Handlers<T>::ssLoadImpl(bufferReader, obj);
}

// ssLoad.  3) Tuple
template<typename T,
         typename std::enable_if<
             !can_ssLoadImpl<T&, BufferReader&>::value &&
             !Handlers<T>::value &&
              can_ssTuple<T>::value
             >::type* = nullptr>
void ssLoadImpl(BufferReader& bufferReader, T& obj)
{
    ssLoadImplViaTuple(bufferReader, const_cast_tuple(obj.ssTuple()));
}

// Overload for internal loading with format info (for F1 non-protected calls)
template<typename T>
void ssLoad(BufferReader& bufferReader, T& obj, bool protectedMode, bool isFormatF1)
{
    if (!protectedMode && isFormatF1) {
        // For F1 format in non-protected mode, don't read format marker
        auto temp = construct<T>();
        ssBeforeLoadImpl(temp);

        if constexpr (std::is_class_v<T>) {
            ssLoadImplInternal(bufferReader, temp);
        } else {
            ssLoadImpl(bufferReader, temp);
        }

        ssAfterLoadImpl(temp);
        obj = std::move(temp);
        return;
    }

    // Otherwise use standard logic
    ssLoad(bufferReader, obj, protectedMode);
}

template<typename T>
void ssLoad(BufferReader& bufferReader, T& obj, bool protectedMode /*= true*/)
{
    std::unique_ptr<BufferReader> partBufferReaderPtr;
    bool isFormatF1 = false;

    BufferReader& partBufferReader = [&bufferReader, protectedMode, &partBufferReaderPtr, &isFormatF1]() -> BufferReader& {
        if (protectedMode) {
            using HashType = decltype(std::declval<Buffer>().hash());

            if (bufferReader.rest() < sizeof(uint64_t) + sizeof(HashType))
                Internal::throwIntegrity();

            const auto size = bufferReader.read<uint64_t>();
            const auto hash = bufferReader.read<HashType>();

            if (bufferReader.rest() < size)
                Internal::throwIntegrity();

            if (size > std::numeric_limits<size_t>::max())
                Internal::throwTooLarge();

            partBufferReaderPtr = std::make_unique<BufferReader>(bufferReader.readRaw(size));

            // Validate hash using new algorithm first, fallback to legacy
            std::optional<bool> optIsNewAlgo {};
            if (hash == partBufferReaderPtr->hash()) {
                optIsNewAlgo = true;
            } else {
                // Try legacy hash computation
                const auto legacyHash = ssHashRaw_F0(partBufferReaderPtr->data(), partBufferReaderPtr->size());
                if (hash == legacyHash) {
                    optIsNewAlgo = false;
                }
            }

            if (optIsNewAlgo) { // Any valid format is accepted
                return *partBufferReaderPtr;
            } else {
                Internal::throwIntegrity();
            }
        } else {
            return bufferReader;
        }
    }();

    if (protectedMode) {
        // Read and validate reserved signature only in protected mode
        uint8_t reserved[Internal::SS_FORMAT_MARK_SIZE];
        partBufferReader.readRaw(reserved, sizeof(reserved));

        const bool isFormatF0Marker = memcmp(reserved, Internal::SS_FORMAT_F0, Internal::SS_FORMAT_MARK_SIZE) == 0;
        const bool isFormatF1Marker = !isFormatF0Marker && (memcmp(reserved, Internal::SS_FORMAT_F1, Internal::SS_FORMAT_MARK_SIZE) == 0);

        if (!isFormatF0Marker && !isFormatF1Marker)
            Internal::throwFormat();

        isFormatF1 = isFormatF1Marker;
    }

    if constexpr (std::is_class_v<T>) {
        if (protectedMode && !isFormatF1) {
            // Format F0, single-version, old hash algorithm (legacy format)
            Internal::LegacyFormatScope legacyScope(Internal::FormatType::Binary, true);  // Set F0 format for this entire operation
            auto temp = construct<T>();
            ssBeforeLoadImpl(temp);

            uint8_t version {};
            partBufferReader.read(version);
            ssLoadAndConvert(partBufferReader, temp, version);

            ssAfterLoadImpl(temp);
            obj = std::move(temp);

        } else {
            // Format F1 or unknown format - use internal load logic
            ssLoadInternal(partBufferReader, obj);
        }
    } else {
        // Primitive type
        auto temp = construct<T>();
        ssBeforeLoadImpl(temp);
        ssLoadImpl(partBufferReader, temp);
        ssAfterLoadImpl(temp);
        obj = std::move(temp);
    }
}

template<typename T>
void ssLoad(BufferReader&& bufferReader, T& obj, bool protectedMode = true)
{
    ssLoad(static_cast<BufferReader&>(bufferReader), obj, protectedMode);
}

template<typename T>
void ssLoad(const Buffer& buffer, T& obj, bool protectedMode = true)
{
    ssLoad(BufferReader(buffer), obj, protectedMode);
}

template<typename T>
[[nodiscard]] T ssLoadRet(BufferReader& bufferReader, bool protectedMode /*= true*/)
{
    auto result = construct<T>();
    ssLoad(bufferReader, result, protectedMode);
    return result;
}

template<typename T>
[[nodiscard]] T ssLoadRet(BufferReader&& bufferReader, bool protectedMode = true)
{
    return ssLoadRet<T>(static_cast<BufferReader&>(bufferReader), protectedMode);
}

template<typename T>
[[nodiscard]] T ssLoadRet(const Buffer& buffer, bool protectedMode = true)
{
    return ssLoadRet<T>(BufferReader(buffer), protectedMode);
}

// Internal load function for F1 format (no format markers)
template<typename T>
void ssLoadInternal(BufferReader& bufferReader, T& obj)
{
    // If we're in F0 format mode, use the F0 logic instead
    if (Internal::isProcessingLegacyFormat(Internal::FormatType::Binary)) {
        obj = ssLoadInternalRet<T>(bufferReader);
        return;
    }

    auto temp = construct<T>();
    ssBeforeLoadImpl(temp);

    if constexpr (std::is_class_v<T>) {
        uint8_t segmentsCount {};
        bufferReader.read(segmentsCount);

        const auto desiredVersion = SSVersion<T>::value;

        // Iterate over segments. First segment contains highest version.
        bool loaded = false;
        uint8_t i = 0;
        for (i = 0; i < segmentsCount; ++i) {
            uint8_t storedVersion {};
            bufferReader.read(storedVersion);
            const auto segmentSize = bufferReader.read<uint64_t>();
            auto segmentData = bufferReader.readRaw(segmentSize);

            if (!loaded) {
                if (storedVersion == desiredVersion) {
                    // We found the desired version, load it.
                    ssLoadImplInternal(segmentData, temp);
                    loaded = true;
                    break;

                } else if (storedVersion < desiredVersion) {
                    // The highest version is less than desired version, upgrade it.
                    ssLoadAndConvert(segmentData, temp, storedVersion);
                    loaded = true;
                    break;

                } else {
                    // Skip segment with too high version
                    continue;
                }
            }
        }

        for (++i; i < segmentsCount; ++i) { // Skip remaining segments
            bufferReader.advance(1); // stored version
            const auto segmentSize = bufferReader.read<uint64_t>();
            bufferReader.advance(segmentSize);
        }

        if (!loaded)
            Internal::throwVersionError();

    } else {
        // Primitive types
        ssLoadImpl(bufferReader, temp);
    }

    ssAfterLoadImpl(temp);
    obj = std::move(temp);
}

} // namespace SuitableStruct
