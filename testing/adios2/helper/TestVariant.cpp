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

TEST(ADIOS2HelperTuple, GetByType)
{
    std::tuple<int, double> tpl = {1, 3.3};
    EXPECT_EQ(adios2::helper::GetByType<int>(tpl), 1);
    EXPECT_EQ(adios2::helper::GetByType<double>(tpl), 3.3);
}

TEST(ADIOS2HelperVariant, Ctor)
{
    using MyVariant = variant<int, double>;
    MyVariant var = 1;
    EXPECT_TRUE(mpark::get<int>(var) == 1);
    var = 3.3;
    EXPECT_EQ(mpark::get<double>(var), 3.3);
}

struct Visitor1
{
  template<class T>
  std::string operator()(const T& val)
  {
    return std::to_string(val);
  }
};

TEST(ADIOS2HelperVariant, Visit)
{
    using MyVariant = variant<int, double>;
    MyVariant var = 3.3;
    EXPECT_EQ(visit(Visitor1{}, var), "3.300000");
    var = 2;
    EXPECT_EQ((visit(Visitor1{}, var)), "2");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    return result;
}
