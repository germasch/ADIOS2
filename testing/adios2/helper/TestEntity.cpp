/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */

#include <iostream>

#include <adios2/helper/TypeList.h>
#include <adios2/helper/adiosType.h>

#include <gtest/gtest.h>

#include "adios2/helper/variant.hpp"
using mpark::variant;

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

using VarTypes = tl::List<int, double>;

namespace detail
{
template <size_t N>
struct int_tag
{
};

template <class Ret, class F, class T>
static Ret do_call(T &&value, F &&f, VarBase &var,
                   int_tag<tl::Size<VarTypes>::value>)
{
    // done trying all, should never get here
    assert(0);
}

template <class Ret, class F, class T, size_t I>
static Ret do_call(T &&value, F &&f, VarBase &var, int_tag<I>)
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

template <class F>
struct ReturnValue
{
    using FirstType = tl::At<0, VarTypes>;
    // FIXME, should check all overloads return same type
    using type = typename std::result_of<F(Var<FirstType> &)>::type;
};

template <class F, class Ret = typename ReturnValue<F>::type>
static Ret visit(VarBase &var, F &&f)
{
    return detail::do_call<Ret>(var.m_Type, std::forward<F>(f), var,
                                detail::int_tag<0>{});
}

} // end namespace detail

#define DECLTYPE_AUTO auto
#define DECLTYPE_AUTO_RETURN(...)                                              \
    ->decltype(__VA_ARGS__) { return __VA_ARGS__; }

template <class F, class FirstType = tl::At<0, VarTypes>,
          class Ret = typename std::result_of<F(Var<FirstType> &)>::type>
static DECLTYPE_AUTO visit(VarBase &var, F &&f)
    DECLTYPE_AUTO_RETURN(detail::visit(var, std::forward<F>(f)))

        struct VarWrapped
{
    // makes a std::variant<std::reference_wrapper<Var<int>>, ...>
    template <class T>
    using VarRef = std::reference_wrapper<Var<T>>;
    using VarRefVariant = tl::Apply<variant, tl::Transform<VarRef, VarTypes>>;

    struct MakeVariant
    {
        template <class T>
        VarRefVariant operator()(Var<T> &var)
        {
            return {var};
        }
    };

    VarWrapped(VarBase &var)
      : m_VarBase(var), m_Variant(adios2::visit(var, MakeVariant{})) // FIXME NS
    {
    }

    struct GetDataType
    {
        template <typename T>
        DataType operator()(VarRef<T> var)
        {
            return helper::GetType<T>();
        }
    };

    DataType Type() const
    {
        // return m_VarBase.m_Type;
        return visit(GetDataType{});
    }

    template <class T>
    Var<T> &Get()
    {
        // return dynamic_cast<Var<T>&>(m_VarBase);
        return mpark::get<VarRef<T>>(m_Variant);
    }

    // FIXME, not const correct?
    template <class F, class Ret = mpark::lib::invoke_result_t<F, VarRef<int>>>
    Ret visit(F &&f) const
    {
        return mpark::visit(std::forward<F>(f), m_Variant);
    }

    VarBase &m_VarBase;
    VarRefVariant m_Variant;
};

}

using namespace adios2;

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
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    return result;
}
