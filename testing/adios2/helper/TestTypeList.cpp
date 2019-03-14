/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */

#include <iostream>
#include <type_traits>
#include <tuple>

#include <adios2/helper/TypeList.h>

#include <gtest/gtest.h>

namespace tl = adios2::helper::tl;

// These are actually compile time tests, so gtest isn't really doing
// anything...

TEST(ADIOS2HelperTypeList, List)
{
    using MyList = tl::List<int, double>;
    static_assert(std::is_same<MyList, tl::List<int, double>>::value,
                  "MyList doesn't match");
}

TEST(ADIOS2HelperTypeList, Apply)
{
    using MyList = tl::List<int, double>;
    using MyTuple = tl::Apply<std::tuple, MyList>;
    static_assert(std::is_same<MyTuple, std::tuple<int, double>>::value,
                  "MyTuple doesn't match");
}

TEST(ADIOS2HelperTypeList, PushFront)
{
    using MyList = tl::List<int, double>;
    using MyList2 = tl::PushFront<float, MyList>;
    static_assert(std::is_same<MyList2, tl::List<float, int, double>>::value,
                  "MyList2 doesn't match");
}

TEST(ADIOS2HelperTypeList, Transform)
{
    using MyList = tl::List<int, double>;
    using MyList2 = tl::Transform<std::vector, MyList>;
    static_assert(
        std::is_same<MyList2,
                     tl::List<std::vector<int>, std::vector<double>>>::value,
        "MyList2 doesn't match");
}

int main(int argc, char **argv)
{

    int result;
    ::testing::InitGoogleTest(&argc, argv);
    result = RUN_ALL_TESTS();

    return result;
}
