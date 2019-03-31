/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * BufferSTL.cpp
 *
 *  Created on: Sep 26, 2017
 *      Author: William F Godoy godoywf@ornl.gov
 */

#include "BufferSTL.h"

namespace adios2
{

void BufferSTL::Resize(const size_t size, const std::string hint)
{
    try
    {
        // doing this will effectively replace the STL GNU default power of 2
        // reallocation.
        m_Buffer.reserve(size);
        // must initialize memory (secure)
        m_Buffer.resize(size, '\0');
    }
    catch (...)
    {
        std::throw_with_nested(std::runtime_error(
            "ERROR: buffer overflow when resizing to " + std::to_string(size) +
            " bytes, " + hint + "\n"));
    }
}

size_t BufferSTL::GetAvailableSize() const
{
    return m_Buffer.size() - m_Position;
}

void BufferSTL::AbsoluteOffsetReset() { m_AbsoluteOffset = 0; }

const std::vector<char> &BufferSTL::Buffer() const
{
    assert(!m_FileDataManager);
    return m_Buffer;
}

std::vector<char> &BufferSTL::Buffer()
{
    assert(!m_FileDataManager);
    return m_Buffer;
}

void BufferSTL::AbsoluteOffsetInc(const size_t offset)
{
    m_AbsoluteOffset += offset;
}

void BufferSTL::WriteFiles(const int transportIndex)
{
    assert(m_FileDataManager);
    m_FileDataManager->WriteFiles(data(), size(), transportIndex);
}

void BufferSTL::FlushFiles(const int transportIndex)
{
    assert(m_FileDataManager);
    m_FileDataManager->FlushFiles(transportIndex);
}

} // end namespace adios2
