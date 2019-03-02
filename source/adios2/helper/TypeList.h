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

/**
 * Apply
 */

namespace detail
{

template <template <class...> class A, class L>
struct Apply
{
};

template <template <class...> class A, template <class...> class L, class... Ts>
struct Apply<A, L<Ts...>>
{
    using type = A<Ts...>;
};
} // end namespace detail

template <template <class...> class A, class L>
using Apply = typename detail::Apply<A, L>::type;

} // end namespace tl
} // end namespace helper
} // end namespace adios2

#endif /* ADIOS2_HELPER_TYPELIST_H_ */
