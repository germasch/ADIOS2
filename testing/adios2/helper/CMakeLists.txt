#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- #
#Distributed under the OSI - approved Apache License, Version 2.0. See
#accompanying file Copyright.txt for details.
#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- #

add_executable(TestHelperString TestHelperStrings.cpp)
target_link_libraries(TestHelperString adios2 gtest)
gtest_add_tests(TARGET TestHelperString ${extra_test_args})

add_executable(TestTypeList TestTypeList.cpp)
target_link_libraries(TestTypeList adios2 gtest)
#gtest_add_tests(TARGET TestTypeList ${extra_test_args})

add_executable(TestVariant TestVariant.cpp)
target_link_libraries(TestVariant adios2 gtest)
#gtest_add_tests(TARGET TestTypeList ${extra_test_args})

add_executable(TestEntity TestEntity.cpp)
target_link_libraries(TestEntity adios2 gtest)
#gtest_add_tests(TARGET TestTypeList ${extra_test_args})
