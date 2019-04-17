#------------------------------------------------------------------------------#
# Distributed under the OSI-approved Apache License, Version 2.0.  See
# accompanying file Copyright.txt for details.
#------------------------------------------------------------------------------#


add_executable(TestNull2Write TestNull2Write.cpp)
target_link_libraries(TestNull2Write adios2 gtest)

if(ADIOS2_HAVE_MPI)
  target_link_libraries(TestNull2Write MPI::MPI_C)
  
  set(extra_test_args EXEC_WRAPPER ${MPIEXEC_COMMAND})
endif()

gtest_add_tests(TARGET TestNull2Write ${extra_test_args} WORKING_DIRECTORY)
