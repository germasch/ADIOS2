/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */

#include <iostream>

#include <adios2/helper/TypeList.h>

#include "/Users/kai/build/ADIOS2/source/adios2/core/variant/include/mapbox/variant.hpp"
using mapbox::util::variant;

#include <gtest/gtest.h>

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
    EXPECT_EQ(var, 1);
    var = 3.3;
    EXPECT_EQ(var, 3.3);
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
    EXPECT_EQ(apply_visitor(Visitor1{}, var), "3.300000");
    var = 2;
    EXPECT_EQ(apply_visitor(Visitor1{}, var), "2");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    return result;
}
