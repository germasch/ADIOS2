/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */

#include <iostream>

#include <adios2/core/Entity.h>

#include <gtest/gtest.h>

using namespace adios2;

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

using VarTypes = helper::tl::List<int, double>;

using VarWrapper = core::EntityWrapper<VarBase, Var, VarTypes>;

struct AsString
{
  template<class T>
  std::string operator()(Var<T>& var)
  {
    return std::to_string(var.m_Value);
  }
};

TEST(ADIOS2HelperVarWrapper, Ctor)
{
    auto var_int = Var<int>{2};
    auto var_double = Var<double>{3.3};

    EXPECT_EQ(var_int.m_Type, DataType::Int32);
    EXPECT_EQ(var_double.m_Type, DataType::Double);

    auto wrap_int = VarWrapper{var_int};
    auto wrap_double = VarWrapper{var_double};

    EXPECT_EQ(wrap_int.Type(), DataType::Int32);
    EXPECT_EQ(wrap_double.Type(), DataType::Double);

    EXPECT_EQ(&wrap_int.Get<int>(), &var_int);
    EXPECT_EQ(&wrap_double.Get<double>(), &var_double);

    EXPECT_EQ(wrap_int.Visit(AsString{}), "2");
    EXPECT_EQ(wrap_double.Visit(AsString{}), "3.300000");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    return result;
}
