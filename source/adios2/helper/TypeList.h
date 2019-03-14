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

} // end namespace tl
} // end namespace helper
} // end namespace adios2

#endif /* ADIOS2_HELPER_TYPELIST_H_ */
