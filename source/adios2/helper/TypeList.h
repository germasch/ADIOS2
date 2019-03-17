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
 * Size
 */
 
template <typename L>
struct Size;

template <template <typename...> class L, typename... Ts>
struct Size<L<Ts...>> {
  static constexpr std::size_t value = sizeof...(Ts);
  using type = std::integral_constant<std::size_t, sizeof...(Ts)>;
};

/**
 * At
 */
 
template< std::size_t I, class L>
struct At;
 
  template< std::size_t I, class Head, class... Tail, template <typename...> class L>
struct At<I, L<Head, Tail...>>
    : At<I-1, L<Tail...>> { };
 
  template< class Head, class... Tail, template <typename...> class L >
struct At<0, L<Head, Tail...>> {
   using type = Head;
};

/**
 * IndexOf
 */

template <class T, std::size_t N, class... Args>
struct IndexOf
{
    static_assert(!std::is_same<T, T>::value, "IndexOf: type not in list");
};

template <class T, std::size_t N, class... Args>
struct IndexOf<T, N, T, Args...>
{ // first element of Args matches T: done
    static constexpr auto value = N;
};

template <class T, std::size_t N, class U, class... Args>
struct IndexOf<T, N, U, Args...>
{
    // inc N, drop U
    static constexpr auto value = IndexOf<T, N + 1, Args...>::value;
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

template <typename L>
using Size = typename detail::Size<L>::type;

template <std::size_t I, typename L>
using At = typename detail::At<I, L>::type;
  
template <class T, class L>
struct IndexOf
{
  static_assert(!std::is_same<T, T>::value, "IndexOf: Type not found!");
};

template <class T, template <class...> class L, class... Args>
struct IndexOf<T, L<Args...>>
{
    static constexpr auto value = detail::IndexOf<T, 0, Args...>::value;
};

} // end namespace tl

/*
 * GetByType
 *
 * Works like std::get, but takes a type rather than an index
 * FIXME, if available (C++14) should just use std::get<type>
 * FIXME, should move into separate header
 */

template <class T, class L>
T &GetByType(L &tpl)
{
    return std::get<tl::IndexOf<T, L>::value>(tpl);
}

} // end namespace helper
} // end namespace adios2

#include "variant.hpp"

	
#endif /* ADIOS2_HELPER_TYPELIST_H_ */
