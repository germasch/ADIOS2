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

} // end namespace helper
} // end namespace adios2

#endif /* ADIOS2_HELPER_TYPELIST_H_ */
