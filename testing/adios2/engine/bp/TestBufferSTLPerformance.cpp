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

TEST_F(TestBufferSTL, Mmap)
{
    double t_beg = MPI_Wtime();
    auto fileDataManager = transportman::TransportMan(MPI_COMM_WORLD, DebugON);
    fileDataManager.OpenFiles({"bufferstl.bin"}, Mode::Write,
			      {{{"transport", "file"}}}, true);
    auto buffer = BufferSTL();
    buffer.m_FileDataManager = &fileDataManager;
    
    const int N = 10 * 1024 * 1024;
    m_Buffer.reserve(N * sizeof(double));

    m_Buffer.resize(N * sizeof(double));
    auto begin = reinterpret_cast<double *>(m_Buffer.data());
    auto end = begin + N;
    std::iota(begin, end, 0);

    m_Buffer.WriteFiles(-1);
    m_FileDataManager.CloseFiles();
    double t_end = MPI_Wtime();
    mprintf("mmap write took %g sec\n", t_end - t_beg);
}

TEST_F(TestBufferSTL, Write)
{
    double t_beg = MPI_Wtime();
    auto fileDataManager = transportman::TransportMan(MPI_COMM_WORLD, DebugON);
    fileDataManager.OpenFiles({"bufferstl.bin"}, Mode::Write,
			      {{{"transport", "file"}}}, true);
    auto buffer = BufferSTL();
    
    const int N = 10 * 1024 * 1024;
    m_Buffer.reserve(N * sizeof(double));

    m_Buffer.resize(N * sizeof(double));
    auto begin = reinterpret_cast<double *>(m_Buffer.data());
    auto end = begin + N;
    std::iota(begin, end, 0);

    m_FileDataManager.WriteFiles(m_Buffer.data(), m_Buffer.size());
    m_FileDataManager.CloseFiles();
    double t_end = MPI_Wtime();
    mprintf("mmap write took %g sec\n", t_end - t_beg);
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
