/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <tuple>
#include <type_traits>
#include <functional>
#include <memory>
#include <optional>
#include <SuitableStruct/Handlers.h>

namespace SuitableStruct {

// Tag type for serializer-specific constructors
struct SS_SERIALIZER_TAG { };

namespace Internal {

enum class FormatType {
    Binary,
    Json
};

std::optional<bool> isProcessingLegacyFormatOpt(FormatType formatType);
bool isProcessingLegacyFormat(FormatType formatType);

class LegacyFormatScope {
public:
    explicit LegacyFormatScope(FormatType formatType, bool isLegacyFormat);
    ~LegacyFormatScope();

    LegacyFormatScope(const LegacyFormatScope&) = delete;
    LegacyFormatScope& operator=(const LegacyFormatScope&) = delete;

private:
    FormatType m_formatType;
    std::optional<bool> m_previousBinaryState;
    std::optional<bool> m_previousJsonState;
};

} // namespace Internal

template<typename T1, typename T2>
struct tuple_cat
{
};

template<typename... T1, typename... T2>
struct tuple_cat<std::tuple<T1...>, std::tuple<T2...>>
{
    using type = std::tuple<T1..., T2...>;
};

template<typename T>
struct const_cast_tuple_helper
{
};

template<>
struct const_cast_tuple_helper<std::tuple<>>
{
    using type = std::tuple<>;
};

template<typename Arg0, typename... Args>
struct const_cast_tuple_helper<std::tuple<Arg0, Args...>>
{
    using type = typename tuple_cat<
        std::tuple<std::add_lvalue_reference_t<std::remove_const_t<std::decay_t<Arg0>>>>,
        typename const_cast_tuple_helper<std::tuple<Args...>>::type
    >::type;
};

template<typename... Tp>
auto& const_cast_tuple(const std::tuple<Tp...>& values)
{
    return ((typename const_cast_tuple_helper<std::tuple<Tp...>>::type&)values);
}

template<int I, typename T, typename T2>
struct tuple_type_index_impl
{
};

template<typename... Args, typename T2>
struct tuple_type_index_impl<-1, std::tuple<Args...>, T2>
{
    static constexpr int value = -1;
};

template<int I, typename... Args, typename T2>
struct tuple_type_index_impl<I, std::tuple<Args...>, T2>
{
    static constexpr int value = std::is_same_v<std::tuple_element_t<I, std::tuple<Args...>>, T2> ? I : tuple_type_index_impl<I-1, std::tuple<Args...>, T2>::value;
};

template<typename T, typename T2>
struct tuple_type_index : public tuple_type_index_impl<-1, T, T2> { };

template<typename... Args, typename T2>
struct tuple_type_index<std::tuple<Args...>, T2> : public tuple_type_index_impl<sizeof...(Args) - 1, std::tuple<Args...>, T2> { };

template <typename T, typename = std::void_t<>>
struct is_std_hashable : std::false_type { };

template <typename T>
struct is_std_hashable<T, std::void_t<decltype(std::declval<std::hash<T>>()(std::declval<T>()))>> : std::true_type { };

template <typename T>
constexpr bool is_std_hashable_v = is_std_hashable<T>::value;

namespace details {
    template <template <class...> class Z, class...>
    struct can_apply : std::false_type { };

    template <template <class...> class Z, class... Ts>
    struct can_apply<Z, std::void_t<Z<Ts...>>, Ts...> : std::true_type { };
} // namespace details

template <template <class...> class Z, class... Ts>
using can_apply = details::can_apply<Z, void, Ts...>;

#define DECLARE_MEMBER_FUNCTION_TESTER(name) \
    template <class F, class... Ts> \
    using name##_r_class = decltype(std::declval<F>(). name( std::declval<Ts>()... )); \
    template<class F, class...Ts> \
    using can_##name = can_apply<name##_r_class, F, Ts...>;

#define CREATE_CHECK_SUBTYPE(functionName) \
template <class T> \
struct has_##functionName \
{ \
    template <typename C> static std::true_type check(typename C::functionName*); \
    template <typename> static std::false_type check(...); \
    static bool const value = std::is_same<decltype(check<T>(0)), std::true_type>::value; \
}; \
template<class T> struct functionName##_helper: std::integral_constant<bool, has_##functionName<T>::value> {}; \
template<class T> static constexpr bool has_##functionName##_v = functionName##_helper<T>::value; \

CREATE_CHECK_SUBTYPE(ssVersions);

DECLARE_MEMBER_FUNCTION_TESTER(ssLoadImpl)
DECLARE_MEMBER_FUNCTION_TESTER(ssSaveImpl)
DECLARE_MEMBER_FUNCTION_TESTER(ssHashImpl)
DECLARE_MEMBER_FUNCTION_TESTER(ssTuple)
DECLARE_MEMBER_FUNCTION_TESTER(size)

DECLARE_MEMBER_FUNCTION_TESTER(ssNamesTuple)
DECLARE_MEMBER_FUNCTION_TESTER(ssJsonLoadImpl)
DECLARE_MEMBER_FUNCTION_TESTER(ssJsonSaveImpl)

DECLARE_MEMBER_FUNCTION_TESTER(ssBeforeSaveImpl)
DECLARE_MEMBER_FUNCTION_TESTER(ssAfterSaveImpl)
DECLARE_MEMBER_FUNCTION_TESTER(ssBeforeLoadImpl)
DECLARE_MEMBER_FUNCTION_TESTER(ssAfterLoadImpl)

// Before/After save/load hooks
template<typename T,
         typename std::enable_if<can_ssBeforeSaveImpl<T>::value>::type* = nullptr>
void ssBeforeSaveImpl(const T& obj)
{
    obj.ssBeforeSaveImpl();
}

template<typename T,
         typename std::enable_if<!can_ssBeforeSaveImpl<T>::value>::type* = nullptr>
void ssBeforeSaveImpl(const T&)
{
}

template<typename T,
         typename std::enable_if<can_ssAfterSaveImpl<T>::value>::type* = nullptr>
void ssAfterSaveImpl(const T& obj)
{
    obj.ssAfterSaveImpl();
}

template<typename T,
         typename std::enable_if<!can_ssAfterSaveImpl<T>::value>::type* = nullptr>
void ssAfterSaveImpl(const T&)
{
}

template<typename T,
         typename std::enable_if<can_ssBeforeLoadImpl<T>::value>::type* = nullptr>
void ssBeforeLoadImpl(T& obj)
{
    obj.ssBeforeLoadImpl();
}

template<typename T,
         typename std::enable_if<!can_ssBeforeLoadImpl<T>::value>::type* = nullptr>
void ssBeforeLoadImpl(T&)
{
}

template<typename T,
         typename std::enable_if<can_ssAfterLoadImpl<T>::value>::type* = nullptr>
void ssAfterLoadImpl(T& obj)
{
    obj.ssAfterLoadImpl();
}

template<typename T,
         typename std::enable_if<!can_ssAfterLoadImpl<T>::value>::type* = nullptr>
void ssAfterLoadImpl(T&)
{
}

// Helper function for constructing objects with serializer tag if available
template<typename T>
[[nodiscard]] T construct()
{
    if constexpr (std::is_constructible_v<T, SS_SERIALIZER_TAG>) {
        return T(SS_SERIALIZER_TAG{});
    } else {
        return T{};
    }
}

template<typename T>
[[nodiscard]] std::unique_ptr<T> construct_unique()
{
    if constexpr (std::is_constructible_v<T, SS_SERIALIZER_TAG>) {
        return std::make_unique<T>(SS_SERIALIZER_TAG{});
    } else {
        return std::make_unique<T>();
    }
}

// Check if type has ssUpgradeFrom in itself
template<typename T, typename T2, typename = void>
struct HasSSUpgradeFromInType : std::false_type {};

template<typename T, typename T2>
struct HasSSUpgradeFromInType<T, T2, std::void_t<decltype(std::declval<T>().ssUpgradeFrom(std::declval<T2>()))>> : std::true_type {};

// Check if type has ssUpgradeFrom in its Handlers
template<typename T, typename T2, typename = void>
struct HasSSUpgradeFromInHandlers : std::false_type {};

template<typename T, typename T2>
struct HasSSUpgradeFromInHandlers<T, T2, std::void_t<decltype(Handlers<T>::ssUpgradeFrom(std::declval<T2>(), std::declval<T&>()))>> : std::true_type {};

// Check if type has ssDowngradeTo in itself
template<typename T, typename T2, typename = void>
struct HasSSDowngradeToInType : std::false_type {};

template<typename T, typename T2>
struct HasSSDowngradeToInType<T, T2, std::void_t<decltype(std::declval<T>().ssDowngradeTo(std::declval<T2&>()))>> : std::true_type {};

// Check if type has ssDowngradeTo in its Handlers
template<typename T, typename T2, typename = void>
struct HasSSDowngradeToInHandlers : std::false_type {};

template<typename T, typename T2>
struct HasSSDowngradeToInHandlers<T, T2, std::void_t<decltype(Handlers<T>::ssDowngradeTo(std::declval<T>(), std::declval<T2&>()))>> : std::true_type {};

// Pre/Post operation method definitions
#define SS_DEFINE_BEFORE_SAVE_CONST() \
    void ssBeforeSaveImpl() const

#define SS_DEFINE_AFTER_SAVE_CONST() \
    void ssAfterSaveImpl() const

#define SS_DEFINE_BEFORE_LOAD() \
    void ssBeforeLoadImpl()

#define SS_DEFINE_AFTER_LOAD() \
    void ssAfterLoadImpl()

} // namespace SuitableStruct
