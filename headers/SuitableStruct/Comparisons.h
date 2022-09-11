#pragma once
#include <tuple>
#include <cmath>
#include <numeric>
#include <memory>

/*
  Fast way to add all comparison operators for custom struct.

  All floating-point numbers already handled.

  Global comparison operators are re-used for all types in struct,
  but any of them could be overriden for your custom struct context.


    Example
  -----------

  struct My_Struct
  {
      int a_;
      double b_;
      char* s_;

      auto ssTuple() const { return std::tie(a_, b_, s_); }
      SS_COMPARISONS_MEMBER(My_Struct); // -- Comparison operators as members
  };


  // Custom comparer example:  strcmp instead of raw pointers comparison.
  //                           Notice. It's non-global, but only for My_Struct.
  inline int compare(const My_Struct&, const char* a, const char* b)
  {
      return strcmp(a, b);
  }

  inline bool compare_eq(const My_Struct&, const char* a, const char* b)
  {
      return strcmp(a, b) == 0;
  }

  // Comparison operators as non-members
  SS_COMPARISONS(My_Struct);    // All:  <, <=, ==, !=, >, >=
  SS_COMPARISONS_EQ(My_Struct); // Only: ==, !=
*/

template<typename T>
int ssThreeWayCompare(const T& a, const T& b)
{
    return (a > b) ?  1 :
           (a < b) ? -1 :
                      0;
}

namespace SuitableStructInternal {

template<typename Struct, typename T>
typename std::enable_if<!std::is_floating_point<T>::value, int>::type
inline compare(const Struct&, const T& a, const T& b)
{
    return ssThreeWayCompare(a, b);
}

/*
template<typename Struct>
inline int compare(const Struct&, const char* a, const char* b)
{
    return (a == b) ? 0 : strcmp(a, b);
}
*/

template <typename Struct, typename T>
typename std::enable_if<std::is_floating_point<T>::value, int>::type
inline compare(const Struct&, T a, T b)
{
    if (std::fabs(a - b) <= std::numeric_limits<T>::epsilon()) {
        return 0;
    } else {
        return (a > b) ? 1 : -1;
    }
}

template<typename Struct, std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), int>::type
compare(const Struct&, const std::tuple<Tp...>&, const std::tuple<Tp...>&)
{
    return 0;
}

template<typename Struct, std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), int>::type
compare(const Struct& s, const std::tuple<Tp...>& t1, const std::tuple<Tp...>& t2)
{
    auto result = compare(s, std::get<I>(t1), std::get<I>(t2));

    return result ? result :
                    compare<Struct, I + 1, Tp...>(s, t1, t2);
}

template<typename Struct, typename T>
inline int compare(const Struct& s, const std::shared_ptr<T>& a, const std::shared_ptr<T>& b)
{
    if (a && b) {
        return compare(s, *a, *b);
    } else {
        return a ?  1 :
               b ? -1 :
                    0;
    }
}

template<typename Struct, typename T>
inline int compare(const Struct& s, const std::unique_ptr<T>& a, const std::unique_ptr<T>& b)
{
    if (a && b) {
        return compare(s, *a, *b);
    } else {
        return a ?  1 :
               b ? -1 :
                    0;
    }
}

template<typename Struct, typename T>
inline int compare(const Struct& s, const std::weak_ptr<T>& weakA, const std::weak_ptr<T>& weakB)
{
    return compare(s, weakA.lock(), weakB.lock());
}

template<typename Struct, typename... Tp>
inline int compareTuples(const Struct& s, const std::tuple<Tp...>& lhs, const std::tuple<Tp...>& rhs)
{
    return compare(s, lhs, rhs);
}

//
// Eq-only
//

template<typename Struct, typename T>
typename std::enable_if<!std::is_floating_point<T>::value, bool>::type
inline compare_eq(const Struct&, const T& a, const T& b)
{
    return (a == b);
}

/*
template<typename Struct>
inline bool compare_eq(const Struct&, const char* a, const char* b)
{
    return strcmp(a, b) == 0;
}
*/

template <typename Struct, typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
inline compare_eq(const Struct&, T a, T b)
{
    return (std::fabs(a - b) <= std::numeric_limits<T>::epsilon());
}

template<typename Struct, std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), bool>::type
compare_eq(const Struct&, const std::tuple<Tp...>&, const std::tuple<Tp...>&)
{
    return true;
}

template<typename Struct, std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), bool>::type
compare_eq(const Struct& s, const std::tuple<Tp...>& t1, const std::tuple<Tp...>& t2)
{
    auto result = compare_eq(s, std::get<I>(t1), std::get<I>(t2));

    return result ? compare_eq<Struct, I + 1, Tp...>(s, t1, t2) :
                    result;
}


template<typename Struct, typename T>
inline bool compare_eq(const Struct& s, const std::shared_ptr<T>& a, const std::shared_ptr<T>& b)
{
    if (a && b) {
        return compare_eq(s, *a, *b);
    } else {
        return (!a && !b);
    }
}

template<typename Struct, typename T>
inline bool compare_eq(const Struct& s, const std::unique_ptr<T>& a, const std::unique_ptr<T>& b)
{
    if (a && b) {
        return compare_eq(s, *a, *b);
    } else {
        return (!a && !b);
    }
}

template<typename Struct, typename T>
inline bool compare_eq(const Struct& s, const std::weak_ptr<T>& weakA, const std::weak_ptr<T>& weakB)
{
    return compare_eq(s, weakA.lock(), weakB.lock());
}

template<typename Struct, typename... Tp>
inline bool compareTuplesEq(const Struct& s, const std::tuple<Tp...>& lhs, const std::tuple<Tp...>& rhs)
{
    return compare_eq(s, lhs, rhs);
}

} // namespace SuitableStructInternal

#define SS_COMPARISONS_OP(STRUCT, OP) \
    inline bool operator OP(const STRUCT& lhs, const STRUCT& rhs) \
    { \
        const auto result = SuitableStructInternal::compareTuples(lhs, lhs.ssTuple(), rhs.ssTuple()); \
        return (result OP 0); \
    }

#define SS_COMPARISONS_MEMBER_OP(STRUCT, OP) \
    inline bool operator OP(const STRUCT& rhs) const \
    { \
        const auto result = SuitableStructInternal::compareTuples(*this, ssTuple(), rhs.ssTuple()); \
        return (result OP 0); \
    }

#define SS_COMPARISONS_OP_EQ(STRUCT, OP) \
    inline bool operator OP(const STRUCT& lhs, const STRUCT& rhs) \
    { \
        const auto result = SuitableStructInternal::compareTuplesEq(lhs, lhs.ssTuple(), rhs.ssTuple()); \
        return (result OP true); \
    }

#define SS_COMPARISONS_MEMBER_OP_EQ(STRUCT, OP) \
    inline bool operator OP(const STRUCT& rhs) const \
    { \
        const auto result = SuitableStructInternal::compareTuplesEq(*this, ssTuple(), rhs.ssTuple()); \
        return (result OP true); \
    }

#define SS_COMPARISONS(STRUCT) \
    SS_COMPARISONS_OP(STRUCT, ==) \
    SS_COMPARISONS_OP(STRUCT, !=) \
    SS_COMPARISONS_OP(STRUCT, <) \
    SS_COMPARISONS_OP(STRUCT, <=) \
    SS_COMPARISONS_OP(STRUCT, >) \
    SS_COMPARISONS_OP(STRUCT, >=)

#define SS_COMPARISONS_MEMBER(STRUCT) \
    SS_COMPARISONS_MEMBER_OP(STRUCT, ==) \
    SS_COMPARISONS_MEMBER_OP(STRUCT, !=) \
    SS_COMPARISONS_MEMBER_OP(STRUCT, <) \
    SS_COMPARISONS_MEMBER_OP(STRUCT, <=) \
    SS_COMPARISONS_MEMBER_OP(STRUCT, >) \
    SS_COMPARISONS_MEMBER_OP(STRUCT, >=)

#define SS_COMPARISONS_ONLY_EQ(STRUCT) \
    SS_COMPARISONS_OP_EQ(STRUCT, ==) \
    SS_COMPARISONS_OP_EQ(STRUCT, !=)

#define SS_COMPARISONS_MEMBER_ONLY_EQ(STRUCT) \
    SS_COMPARISONS_MEMBER_OP_EQ(STRUCT, ==) \
    SS_COMPARISONS_MEMBER_OP_EQ(STRUCT, !=)
