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

template<typename E>
struct Visitation
{
  using E_ = typename std::decay<E>::type;
  using Types = typename E_::Types;
  template<typename T>
  using Entity = typename E_::template Entity<T>;
  // also expects that E has E::Base() and uses E::Type()

  template <size_t N>
  struct int_tag
  {
  };

  template <class Ret, class F, class T>
  static Ret do_call(T &&datatype, E&& entity, F &&f,
		     int_tag<tl::Size<Types>::value>)
  {
    // done trying all, should never get here
    assert(0);
  }

  template <class Ret, class F, class T, size_t I>
  static Ret do_call(T &&datatype, E&& entity, F &&f, int_tag<I>)
  {
    using Type = tl::At<I, Types>;
    if (datatype == helper::GetType<Type>())
    {
      return std::forward<F>(f)(dynamic_cast<Entity<Type> &>(entity.Base()));
    }
    else
    {
      return do_call<Ret>(std::forward<T>(datatype), std::forward<E>(entity), std::forward<F>(f),
                            int_tag<I + 1>{});
    }
  };

  template <class F>
  struct ReturnValue
  {
    using FirstType = tl::At<0, Types>;
    // FIXME, should check all overloads return same type
    using type = typename std::result_of<F(Entity<FirstType> &)>::type;
  };

  template <class F, class Ret = typename ReturnValue<F>::type>
  static Ret visit(E &&entity, F &&f)
  {
    return do_call<Ret>(entity.Type(), std::forward<E>(entity), std::forward<F>(f),
			int_tag<0>{});
  }
};
  
template <class E, class F>
static DECLTYPE_AUTO visit(E &&entity, F &&f)
  DECLTYPE_AUTO_RETURN(Visitation<E>::visit(std::forward<E>(entity),  std::forward<F>(f)))

} // end namespace detail

template <class _EntityBase, template<typename> class _Entity, class _Types>
class EntityWrapper
{
public:
    using EntityBase = _EntityBase;
    using Types = _Types;
    template<typename T>
    using Entity = _Entity<T>;

    EntityWrapper(EntityBase &var)
      : m_Base(var)
    {
    }

    DataType Type() const
    {
        return m_Base.m_Type;
    }

    EntityBase& Base() { return m_Base; }
    const EntityBase& Base() const { return m_Base; }

    template <class T>
    Entity<T> &Get()
    {
        return dynamic_cast<Entity<T>&>(m_Base);
    }

    template <class F>
    DECLTYPE_AUTO Visit(F &&f) DECLTYPE_AUTO_RETURN(detail::visit(*this, std::forward<F>(f)))

private:
    EntityBase &m_Base;
};


} // end namespace core
} // end namespace adios2

/* #undef DECLTYPE_AUTO */
/* #undef DECLTYPE_AUTO_RETURN */

#endif /* ADIOS2_CORE_ENTITY_H_ */
