/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * TypeList.h: simple template metaprogramming library
 *
 *  Created on: Feb 2, 2019
 *      Author: Kai Germaschewski <kai.germaschewski@unh.edu>
 */

#ifndef ADIOS2_HELPER_TYPELIST_H_
#define ADIOS2_HELPER_TYPELIST_H_

namespace adios2
{
namespace helper
{
namespace tl
{

/**
 * List
 */

template <class... Ts>
struct List
{
};

namespace detail
{

/**
 * Apply
 */

template <template <class...> class A, class L>
struct Apply
{
};

template <template <class...> class A, template <class...> class L, class... Ts>
struct Apply<A, L<Ts...>>
{
    using type = A<Ts...>;
};

/**
 * PushFront
 */

template <class T, class L>
struct PushFront;

template <class T, template <class...> class L, class... U>
struct PushFront<T, L<U...>>
{
    using type = L<T, U...>;
};

/**
 * Transform
 */

template <template <class...> class F, class L>
struct Transform;

template <template <class...> class F, template <class...> class L, class... T>
struct Transform<F, L<T...>>
{
    using type = L<F<T>...>;
};

/**
 * GetIndex
 */

/* template <class T, class L> */
/* struct GetIndex; */

template <class T, std::size_t N, class... Args>
struct GetIndex
{
    static_assert(!std::is_same<T, T>::value, "GetIndex: type not in list");
};

template <class T, std::size_t N, class... Args>
struct GetIndex<T, N, T, Args...>
{ // first element of Args matches T: done
    static constexpr auto value = N;
};

template <class T, std::size_t N, class U, class... Args>
struct GetIndex<T, N, U, Args...>
{
    // inc N, drop U
    static constexpr auto value = GetIndex<T, N + 1, Args...>::value;
};

} // end namespace detail

/**
 * shortcuts for detail:: implementations
 */

template <template <class...> class A, class L>
using Apply = typename detail::Apply<A, L>::type;

template <class T, class L>
using PushFront = typename detail::PushFront<T, L>::type;

template <template <class...> class F, class L>
using Transform = typename detail::Transform<F, L>::type;

template <class T, class L>
struct GetIndex
{
  static_assert(!std::is_same<T, T>::value, "GetIndex: Type not found!");
};

template <class T, template <class...> class L, class... Args>
struct GetIndex<T, L<Args...>>
{
    static constexpr auto value = detail::GetIndex<T, 0, Args...>::value;
};

} // end namespace tl

/*
 * get_by_type
 *
 * Works like std::get, but takes a type rather than an index
 * FIXME, if available (C++14) should just use std::get<type>
 * FIXME, should move into separate header
 */

template <class T, class L>
T &GetByType(L &tpl)
{
    return std::get<tl::GetIndex<T, L>::value>(tpl);
}


// ======================================================================
 
#if 0

 
template <class C> 
constexpr auto size(const C& c) -> decltype(c.size())
{
    return c.size();
}
  
// is_finite_set

template <class T>
struct is_finite_set;

namespace detail {
    template <class T, class SFINAE = void>
    struct detect_finite_set : std::false_type {};

    template <class T>
    struct detect_finite_set<T, typename std::enable_if<size(T::value) >= 1>::type> : std::true_type {};
}
template <class T>
struct is_finite_set : detail::detect_finite_set<T> {};


namespace detail {
    template <class FiniteSet, size_t I, class F>
    constexpr void /*decltype(auto)*/ call_with_tags(F&& f) {
      return std::forward<F>(f)(value_tag<FiniteSet, I>{});
    }
}
  
template <class FiniteSet>
struct sequential_search_strategy
{
  using Value = std::string;
  
  static constexpr auto value = is_finite_set<FiniteSet>::value;
  
  template <size_t I>
  struct int_tag {};
  
  template <class Ret, class T, class F>
  static constexpr Ret try_call(T && value, F && f, int_tag<size(FiniteSet::value)>) {
    /* should do nothing */
    return Ret{};
  }
  template <class Ret, class T, class F, size_t I>
  static constexpr Ret try_call(T && value, F && f, int_tag<I>) {
    if (value == std::get<I>(FiniteSet::value)) {
      return detail::call_with_tags<FiniteSet, I>(std::forward<F>(f));
    }
    else {
      return try_call<Ret>(std::foward<Value>(value), std::forward<F>(f), int_tag<I + 1>{});
    }
  }
  
  template <class T, class F>
  constexpr void /*decltype(auto)*/ operator()(T && value, F && f) const {
    using return_type = decltype(detail::call_with_tags<FiniteSet, 0>(std::forward<F>(f)));
      return try_call<return_type>(std::forward<Value>(value), std::forward<F>(f), int_tag<0>{});
  }
};

#endif

} // end namespace helper
} // end namespace adios2

#include "variant.hpp"

	
#endif /* ADIOS2_HELPER_TYPELIST_H_ */
