/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * Entity.h : Wrapper for Variables / Attrbutes
 *
 *  Created on: Mar 15, 2018
 *      Author: Kai Germaschewski <kai.germaschewski@unh.edu>
 */

#ifndef ADIOS2_CORE_ENTITY_H_
#define ADIOS2_CORE_ENTITY_H_

#include "adios2/helper/adiosType.h"
#include <adios2/helper/TypeList.h>

#define DECLTYPE_AUTO auto
#define DECLTYPE_AUTO_RETURN(...)                                              \
    ->decltype(__VA_ARGS__) { return __VA_ARGS__; }

namespace adios2
{
namespace core
{

namespace tl = adios2::helper::tl;

namespace detail
{

template <typename E>
struct Visitation
{
    using E_ = typename std::remove_reference<E>::type;
    using Types = typename E_::Types;
    template <typename T>
    using Entity =
        typename std::conditional<std::is_const<E_>::value,
                                  const typename E_::template Entity<T>,
                                  typename E_::template Entity<T>>::type;
    // also expects that E has E::Base() and uses E::Type()

    template <size_t N>
    struct int_tag
    {
    };

    template <class Ret, class F, class T, class... Args>
    static Ret do_call(T &&datatype, E &&entity, F &&f,
                       int_tag<tl::Size<Types>::value>, Args &&... args)
    {
        // done trying all, should never get here
        assert(0);
    }

    template <class Ret, class F, class T, size_t I, class... Args>
    static Ret do_call(T &&datatype, E &&entity, F &&f, int_tag<I>,
                       Args &&... args)
    {
        using Type = tl::At<I, Types>;
        if (datatype == helper::GetType<Type>())
        {
            return std::forward<F>(f)(
                dynamic_cast<Entity<Type> &>(entity.Base()),
                std::forward<Args>(args)...);
        }
        else
        {
            return do_call<Ret>(std::forward<T>(datatype),
                                std::forward<E>(entity), std::forward<F>(f),
                                int_tag<I + 1>{}, std::forward<Args>(args)...);
        }
    };

    template <class F, class... Args>
    struct ReturnValue
    {
        using FirstType = tl::At<0, Types>;
        // FIXME, should check all overloads return same type
        using type =
            typename std::result_of<F(Entity<FirstType> &, Args &&...)>::type;
    };

    // FIXME, must be possible to do this pretter
    // FIXME, better error when (generally) failing to find correct signature in
    // F
    template <class F, class... Args>
    static typename std::result_of<F && (Entity<tl::At<0, Types>> &,
                                         Args &&...)>::type
    visit(E &&entity, F &&f, Args &&... args)
    {
        return do_call<typename ReturnValue<F &&, Args &&...>::type>(
            entity.Type(), std::forward<E>(entity), std::forward<F>(f),
            int_tag<0>{}, std::forward<Args>(args)...);
    }
};

template <class E, class F, class... Args>
static DECLTYPE_AUTO visit(E &&entity, F &&f, Args &&... args)
    DECLTYPE_AUTO_RETURN(Visitation<E>::visit(std::forward<E>(entity),
                                              std::forward<F>(f),
                                              std::forward<Args>(args)...))

} // end namespace detail

template <class _Base, template <typename> class _Entity, class _Types>
class EntityWrapper : public _Base
{
public:
    using Self = EntityWrapper<_Base, _Entity, _Types>;
    using Types = _Types;
    template <typename T>
    using Entity = _Entity<T>;

    EntityWrapper() = delete;
    EntityWrapper(const EntityWrapper &) = delete;

    DataType Type() const { return _Base::m_Type; }

    _Base &Base() { return *this; }
    const _Base &Base() const { return *this; }

    template <class T>
    Entity<T> &GetAs()
    {
        return dynamic_cast<Entity<T> &>(static_cast<_Base &>(*this));
    }

    template <class T>
    const Entity<T> &GetAs() const
    {
        return dynamic_cast<const Entity<T> &>(
            static_cast<const _Base &>(*this));
    }

    template <class F, class... Args>
    DECLTYPE_AUTO Visit(F &&f, Args &&... args)
        DECLTYPE_AUTO_RETURN(detail::visit(*this, std::forward<F>(f),
                                           std::forward<Args>(args)...));

    template <class F, class... Args>
    DECLTYPE_AUTO Visit(F &&f, Args &&... args) const
        DECLTYPE_AUTO_RETURN(detail::visit(*this, std::forward<F>(f),
                                           std::forward<Args>(args)...));

    static Self &cast(_Base &base) { return reinterpret_cast<Self &>(base); }
    static const Self &cast(const _Base &base)
    {
        return reinterpret_cast<const Self &>(base);
    }
};

} // end namespace core
} // end namespace adios2

/* #undef DECLTYPE_AUTO */
/* #undef DECLTYPE_AUTO_RETURN */

#endif /* ADIOS2_CORE_ENTITY_H_ */
