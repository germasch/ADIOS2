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
    TestBufferSTL() : m_FileDataManager(MPI_COMM_WORLD, DebugON)
    {
        m_FileDataManager.OpenFiles({"bufferstl.bin"}, Mode::Write,
                                    {{{"transport", "file"}}}, true);
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

    m_FileDataManager.CloseFiles();
}

TEST_F(TestBufferSTL, resizeShrink)
{
    const int N = 10000;
    m_Buffer.reserve(20000 * sizeof(int));
    m_Buffer.resize(N * sizeof(int));
    int *begin = reinterpret_cast<int *>(m_Buffer.data());
    int *end = begin + N;
    std::iota(begin, end, 0);

    m_Buffer.resize(N / 2 * sizeof(int));
    m_Buffer.reserve(N / 2 * sizeof(int));

    EXPECT_EQ(m_Buffer.capacity(), N / 2 * sizeof(int));

    m_FileDataManager.CloseFiles();
}

TEST_F(TestBufferSTL, resizeGrowInPlace)
{
    m_Buffer.reserve(16384);
    m_Buffer.reserve(20000);

    EXPECT_EQ(m_Buffer.capacity(), 20000);

    m_FileDataManager.CloseFiles();
}

TEST_F(TestBufferSTL, resizeGrow)
{
    m_Buffer.reserve(16384);
    m_Buffer.reserve(6000000);

    EXPECT_EQ(m_Buffer.capacity(), 6000000);
    // usleep(100000000);
}

TEST_F(TestBufferSTL, write)
{
    const int N = 10;
    m_Buffer.reserve(16384);

    m_Buffer.resize(N * sizeof(int));
    int *begin = reinterpret_cast<int *>(m_Buffer.data());
    int *end = begin + N;
    std::iota(begin, end, 0);

    if (m_Buffer.m_FileDataManager)
    {
        m_Buffer.WriteFiles(-1);
    }
    else
    {
        m_FileDataManager.WriteFiles(m_Buffer.data(), m_Buffer.size());
    }
    m_FileDataManager.CloseFiles();
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
