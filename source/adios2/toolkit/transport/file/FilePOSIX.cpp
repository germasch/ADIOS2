/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * FileDescriptor.cpp file I/O using POSIX I/O library
 *
 *  Created on: Oct 6, 2016
 *      Author: William F Godoy godoywf@ornl.gov
 */
#include "FilePOSIX.h"

#include <fcntl.h>     // open
#include <stddef.h>    // write output
#include <sys/mman.h>  // mmap
#include <sys/stat.h>  // open, fstat
#include <sys/types.h> // open
#include <system_error>
#include <unistd.h> // write, close

/// \cond EXCLUDE_FROM_DOXYGEN
#include <ios> //std::ios_base::failure
/// \endcond

namespace adios2
{
namespace transport
{

FilePOSIX::FilePOSIX(MPI_Comm mpiComm, const bool debugMode)
: Transport("File", "POSIX", mpiComm, debugMode)
{
}

FilePOSIX::~FilePOSIX()
{
    if (m_IsOpen)
    {
        close(m_FileDescriptor);
    }
}

void FilePOSIX::Open(const std::string &name, const Mode openMode)
{
    m_Name = name;
    CheckName();
    m_OpenMode = openMode;

    switch (m_OpenMode)
    {

    case (Mode::Write):
        ProfilerStart("open");
        m_FileDescriptor =
            open(m_Name.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
        ProfilerStop("open");
        break;

    case (Mode::Append):
        ProfilerStart("open");
        // m_FileDescriptor = open(m_Name.c_str(), O_RDWR);
        m_FileDescriptor =
            open(m_Name.c_str(), O_RDWR | O_APPEND | O_CREAT, 0777);
        ProfilerStop("open");
        break;

    case (Mode::Read):
        ProfilerStart("open");
        m_FileDescriptor = open(m_Name.c_str(), O_RDONLY);
        ProfilerStop("open");
        break;

    default:
        CheckFile("unknown open mode for file " + m_Name +
                  ", in call to POSIX open");
    }

    CheckFile("couldn't open file " + m_Name +
              ", check permissions or path existence, in call to POSIX open");

    m_IsOpen = true;
}

void FilePOSIX::Write(const char *buffer, size_t size, size_t start)
{
    mprintf("FilePOSIX::Write size %ld (%p) start %ld\n", size, buffer, start);
    auto lf_Write = [&](const char *buffer, size_t size) {
        while (size > 0)
        {
            ProfilerStart("write");
            const auto writtenSize = write(m_FileDescriptor, buffer, size);
            ProfilerStop("write");

            if (writtenSize == -1)
            {
                if (errno == EINTR)
                {
                    continue;
                }

                throw std::ios_base::failure(
                    "ERROR: couldn't write to file " + m_Name +
                    ", in call to FileDescriptor Write\n");
            }

            buffer += writtenSize;
            size -= writtenSize;
        }
    };

    if (start != MaxSizeT)
    {
        const auto newPosition = lseek(m_FileDescriptor, start, SEEK_SET);

        if (static_cast<size_t>(newPosition) != start)
        {
            throw std::ios_base::failure(
                "ERROR: couldn't move to start position " +
                std::to_string(start) + " in file " + m_Name +
                ", in call to POSIX lseek\n");
        }
    }

    if (size > DefaultMaxFileBatchSize)
    {
        const size_t batches = size / DefaultMaxFileBatchSize;
        const size_t remainder = size % DefaultMaxFileBatchSize;

        size_t position = 0;
        for (size_t b = 0; b < batches; ++b)
        {
            lf_Write(&buffer[position], DefaultMaxFileBatchSize);
            position += DefaultMaxFileBatchSize;
        }
        lf_Write(&buffer[position], remainder);
    }
    else
    {
        lf_Write(buffer, size);
    }
}

void FilePOSIX::Read(char *buffer, size_t size, size_t start)
{
    auto lf_Read = [&](char *buffer, size_t size) {
        while (size > 0)
        {
            ProfilerStart("read");
            const auto readSize = read(m_FileDescriptor, buffer, size);
            ProfilerStop("read");

            if (readSize == -1)
            {
                if (errno == EINTR)
                {
                    continue;
                }

                throw std::ios_base::failure("ERROR: couldn't read from file " +
                                             m_Name +
                                             ", in call to POSIX IO read\n");
            }

            buffer += readSize;
            size -= readSize;
        }
    };

    if (start != MaxSizeT)
    {
        const auto newPosition = lseek(m_FileDescriptor, start, SEEK_SET);

        if (static_cast<size_t>(newPosition) != start)
        {
            throw std::ios_base::failure(
                "ERROR: couldn't move to start position " +
                std::to_string(start) + " in file " + m_Name +
                ", in call to POSIX lseek errno " + std::to_string(errno) +
                "\n");
        }
    }

    if (size > DefaultMaxFileBatchSize)
    {
        const size_t batches = size / DefaultMaxFileBatchSize;
        const size_t remainder = size % DefaultMaxFileBatchSize;

        size_t position = 0;
        for (size_t b = 0; b < batches; ++b)
        {
            lf_Read(&buffer[position], DefaultMaxFileBatchSize);
            position += DefaultMaxFileBatchSize;
        }
        lf_Read(&buffer[position], remainder);
    }
    else
    {
        lf_Read(buffer, size);
    }
}

char *FilePOSIX::Resize(size_t size)
{
    assert(m_IsOpen);
    mprintf("FilePOSIX::Realloc B %#lx (m_Start %p)\n", size, m_MmapStart);
    int rv = ftruncate(m_FileDescriptor, size);
    if (rv < 0)
    {
        perror("ftruncate");
    }

    // if size is smaller than what we already mapped, nothing tbd
    size_t pageSize = sysconf(_SC_PAGE_SIZE);
    size_t newMmapSize = (size + pageSize - 1) & ~(pageSize - 1);
    if (newMmapSize > m_MmapSize)
    {
        if (m_MmapStart)
        {
            // try extending first
            char *currentEnd = m_MmapStart + m_MmapSize;
            size_t addSize = newMmapSize - m_MmapSize;
            mprintf("have %p -> %p\n", m_MmapStart, m_MmapStart + m_MmapSize);
            mprintf("add  %p -> %p off %#lx\n", currentEnd,
                    currentEnd + addSize, m_MmapSize);
            mprintf("FilePOSIX mmap\n");
            char *start = static_cast<char *>(
                mmap(currentEnd, addSize, PROT_READ | PROT_WRITE, MAP_SHARED,
                     m_FileDescriptor, m_MmapSize));
            if (start == MAP_FAILED)
            {
                perror("mmap");
            }
            if (start == currentEnd)
            {
                // success, got it extended
                mprintf("FilePOSIX extend success\n");
                m_MmapSize = newMmapSize;
            }
            else
            {
                // nope, got to start over
                mprintf("FilePOSIX munmap\n");
                munmap(start, addSize); // FIXME, rv
                mprintf("FilePOSIX munmap\n");
                munmap(m_MmapStart, m_MmapSize); // FIXME, rv
                m_MmapStart = nullptr;
                m_MmapSize = 0;
            }
        }

        if (!m_MmapStart)
        {
            m_MmapSize = newMmapSize;
            mprintf("FilePOSIX mmap\n");
            m_MmapStart =
                static_cast<char *>(mmap(0, m_MmapSize, PROT_READ | PROT_WRITE,
                                         MAP_SHARED, m_FileDescriptor, 0));
            if (m_MmapStart == MAP_FAILED)
            {
                perror("mmap");
                throw std::system_error(errno, std::generic_category());
            }
        }
    }
    mprintf("FilePOSIX::Realloc E %#lx (m_Start %p)\n", size, m_MmapStart);
    // usleep(100000000);
    return static_cast<char *>(m_MmapStart);
}

size_t FilePOSIX::GetSize()
{
    struct stat fileStat;
    if (fstat(m_FileDescriptor, &fileStat) == -1)
    {
        throw std::ios_base::failure("ERROR: couldn't get size of file " +
                                     m_Name + "\n");
    }
    return static_cast<size_t>(fileStat.st_size);
}

void FilePOSIX::Flush() {}

void FilePOSIX::Close()
{
    ProfilerStart("close");
    if (m_MmapStart)
    {
        munmap(m_MmapStart, m_MmapSize);
        m_MmapStart = nullptr;
        m_MmapSize = 0;
    }
    const int status = close(m_FileDescriptor);
    ProfilerStop("close");

    if (status == -1)
    {
        throw std::ios_base::failure("ERROR: couldn't close file " + m_Name +
                                     ", in call to POSIX IO close\n");
    }

    m_IsOpen = false;
}

void FilePOSIX::CheckFile(const std::string hint) const
{
    if (m_FileDescriptor == -1)
    {
        throw std::ios_base::failure("ERROR: " + hint + "\n");
    }
}

} // end namespace transport
} // end namespace adios2
