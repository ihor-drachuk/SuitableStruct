#pragma once
#include <tuple>
#include <type_traits>

namespace SuitableStruct {

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

template<typename Arg0>
struct const_cast_tuple_helper<std::tuple<Arg0>>
{
    using type = std::tuple<std::add_lvalue_reference_t<std::remove_const_t<std::decay_t<Arg0>>>>;
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

} // namespace SuitableStruct
