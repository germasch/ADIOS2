/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */

#include <iostream>

#include <adios2/helper/TypeList.h>
#include <adios2/helper/adiosType.h>

#include <gtest/gtest.h>

namespace tl = adios2::helper::tl;

namespace adios2
{

// ======================================================================

#define DECLTYPE_AUTO auto
#define DECLTYPE_AUTO_RETURN(...)                                              \
    ->decltype(__VA_ARGS__) { return __VA_ARGS__; }

namespace helper
{

namespace detail
{

template<typename E>
struct Visitation
{
  using E_ = typename std::decay<E>::type;
  using Types = typename E_::Types;
  template<typename T>
  using Entity = typename E_::template Entity<T>;

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
        return std::forward<F>(f)(dynamic_cast<Entity<Type> &>(entity.m_VarBase));
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
  
} // end namespace detail

template <class E, class F>
static DECLTYPE_AUTO visit(E &&entity, F &&f)
  DECLTYPE_AUTO_RETURN(detail::Visitation<E>::visit(std::forward<E>(entity),  std::forward<F>(f)))

} // end namespace helper

// ======================================================================

struct VarBase
{
    VarBase(DataType type) : m_Type(type) {}
    virtual ~VarBase() = default;

    DataType m_Type;
};

template <class T>
struct Var : VarBase
{
    Var(T t) : VarBase(adios2::helper::GetType<T>()), m_Value(t) {}

    T m_Value;
};

using VarTypes = tl::List<int, double>;

struct VarWrapped
{
    using Base = VarBase;
    using Types = VarTypes;
    template<typename T>
    using Entity = Var<T>;

    VarWrapped(VarBase &var)
      : m_VarBase(var)
    {
    }

    DataType Type() const
    {
        return m_VarBase.m_Type;
    }

    template <class T>
    Var<T> &Get()
    {
        return dynamic_cast<Var<T>&>(m_VarBase);
    }

    // FIXME, not const correct?
    template <class F>
    auto visit(F &&f) const -> std::string
    {
      return helper::visit(*this, std::forward<F>(f));
    }

    VarBase &m_VarBase;
};

}

#undef DECLTYPE_AUTO
#undef DECLTYPE_AUTO_RETURN

using namespace adios2;

struct AsString
{
  template<class T>
  std::string operator()(Var<T>& var)
  {
    return std::to_string(var.m_Value);
  }
};

TEST(ADIOS2HelperVarWrapped, Ctor)
{
    auto var_int = Var<int>{2};
    auto var_double = Var<double>{3.3};

    EXPECT_EQ(var_int.m_Type, DataType::Int32);
    EXPECT_EQ(var_double.m_Type, DataType::Double);

    auto wrap_int = VarWrapped{var_int};
    auto wrap_double = VarWrapped{var_double};

    EXPECT_EQ(wrap_int.Type(), DataType::Int32);
    EXPECT_EQ(wrap_double.Type(), DataType::Double);

    EXPECT_EQ(&wrap_int.Get<int>(), &var_int);
    EXPECT_EQ(&wrap_double.Get<double>(), &var_double);

    EXPECT_EQ(wrap_int.visit(AsString{}), "2");
    EXPECT_EQ(wrap_double.visit(AsString{}), "3.300000");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    return result;
}
