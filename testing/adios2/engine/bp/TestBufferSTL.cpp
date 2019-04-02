#include <cstdint>

#include <iostream>
#include <numeric>
#include <stdexcept>

#include "adios2/toolkit/format/BufferSTL.h"
#include "adios2/toolkit/transportman/TransportMan.h"

#include <gtest/gtest.h>

using namespace adios2;

class TestBufferSTL : public ::testing::Test
{
public:
  TestBufferSTL()
    : m_FileDataManager(MPI_COMM_WORLD, DebugON)
  {
    m_FileDataManager.OpenFiles({"bufferstl.bin"}, Mode::Write, {{{"transport", "file"}}}, true);
    m_Buffer.m_FileDataManager = &m_FileDataManager;
  }

  transportman::TransportMan m_FileDataManager;
  BufferSTL m_Buffer;
};

TEST_F(TestBufferSTL, reserve)
{
  m_Buffer.reserve(16384);

  EXPECT_EQ(m_Buffer.capacity(), 16384);
  EXPECT_EQ(m_Buffer.size(), 0);
}

TEST_F(TestBufferSTL, resize)
{
  BufferSTL m_Buffer;
  m_Buffer.reserve(16384);
  m_Buffer.reserve(20000);

  EXPECT_EQ(m_Buffer.capacity(), 20000);
}

int main(int argc, char **argv)
{
#ifdef ADIOS2_HAVE_MPI
    MPI_Init(nullptr, nullptr);
#endif

    int result;
    ::testing::InitGoogleTest(&argc, argv);
    result = RUN_ALL_TESTS();

#ifdef ADIOS2_HAVE_MPI
    MPI_Finalize();
#endif

    return result;
}
