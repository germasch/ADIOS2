/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */

#include <iostream>

#include <adios2/helper/TypeList.h>

#include <gtest/gtest.h>

using namespace adios2::helper;

// These are actually compile time tests, so gtest isn't really doing
// anything...

TEST(ADIOS2HelperTypeList, List)
{
    using MyList = tl::List<int, double>;
    static_assert(std::is_same<MyList, tl::List<int, double>>::value,
                  "MyList doesn't match");
}

int main(int argc, char **argv)
{

    int result;
    ::testing::InitGoogleTest(&argc, argv);
    result = RUN_ALL_TESTS();

    return result;
}
