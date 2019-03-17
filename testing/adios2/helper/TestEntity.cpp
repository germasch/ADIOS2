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

// ======================================================================

#define DECLTYPE_AUTO auto
#define DECLTYPE_AUTO_RETURN(...)                                              \
    ->decltype(__VA_ARGS__) { return __VA_ARGS__; }

using VarTypes = tl::List<int, double>;

namespace helper
{

template<typename T>
using Entity = Var<T>;
using EntityBase = VarBase;

namespace detail
{
template <size_t N>
struct int_tag
{
};

template <class Ret, class F, class T>
static Ret do_call(T &&value, F &&f, EntityBase &var,
                   int_tag<tl::Size<VarTypes>::value>)
{
    // done trying all, should never get here
    assert(0);
}

template <class Ret, class F, class T, size_t I>
static Ret do_call(T &&value, F &&f, EntityBase &var, int_tag<I>)
{
    using Type = tl::At<I, VarTypes>;
    if (value == helper::GetType<Type>())
    {
        return std::forward<F>(f)(dynamic_cast<Var<Type> &>(var));
    }
    else
    {
        return do_call<Ret>(std::forward<T>(value), std::forward<F>(f), var,
                            int_tag<I + 1>{});
    }
};

template <class E, class F>
struct ReturnValue
{
    using FirstType = tl::At<0, VarTypes>;
    // FIXME, should check all overloads return same type
    using type = typename std::result_of<F(Var<FirstType> &)>::type;
};

  template <class E, class F, class Ret = typename ReturnValue<E, F>::type>
static Ret visit(E &&entity, F &&f)
{
    return detail::do_call<Ret>(entity.Type(), std::forward<F>(f), entity.m_VarBase,
                                detail::int_tag<0>{});
}

} // end namespace detail

template <class E, class F>
static DECLTYPE_AUTO visit(E &&entity, F &&f)
  DECLTYPE_AUTO_RETURN(detail::visit(std::forward<E>(entity),  std::forward<F>(f)))

} // end namespace helper

// ======================================================================

struct VarWrapped
{
    using Base = VarBase;

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
