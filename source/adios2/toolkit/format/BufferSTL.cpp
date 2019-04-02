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

BufferSTL::BufferSTL() : m_Buffer(), m_Data(m_Buffer.data()) {}

size_t BufferSTL::capacity() const
{
    if (m_FileDataManager)
    {
        mprintf("BACK: capacity %ld\n", m_MmapCapacity);
        return m_MmapCapacity;
    }
    return m_Buffer.size();
}

void BufferSTL::Resize(const size_t size, const std::string hint)
{
    if (m_FileDataManager)
    {
        mprintf("BACK: resize %ld\n", size);
        mprintf("BACK: data %p\n", m_Data);
        m_Data = m_FileDataManager->ResizeFiles(size);
        m_MmapCapacity = size;
        return;
    }
    try
    {
        m_Buffer.resize(size);
        m_Data = m_Buffer.data();
    }
    catch (...)
    {
        std::throw_with_nested(std::runtime_error(
            "ERROR: buffer overflow when resizing to " + std::to_string(size) +
            " bytes, " + hint + "\n"));
    }
}

size_t BufferSTL::GetAvailableSize() const { return capacity() - m_Position; }

void BufferSTL::AbsoluteOffsetReset() { m_AbsoluteOffset = 0; }

const std::vector<char> &BufferSTL::Buffer() const
{
    assert(!m_FileDataManager);
    return m_Buffer.Buffer();
}

std::vector<char> &BufferSTL::Buffer()
{
    assert(!m_FileDataManager);
    return m_Buffer.Buffer();
}

void BufferSTL::AbsoluteOffsetInc(const size_t offset)
{
    m_AbsoluteOffset += offset;
}

void BufferSTL::WriteFiles(const int transportIndex)
{
    assert(m_FileDataManager);
    mprintf("BufferSTL::WriteFiles %ld\n", size());
    Resize(size(),
           "in BufferSTL::WriteFiles"); // slightly ugly, and leaves us with the
                                        // wrong capacity going forward
    // m_FileDataManager->WriteFiles(data(), size(), transportIndex);
}

void BufferSTL::FlushFiles(const int transportIndex)
{
    assert(m_FileDataManager);
    mprintf("BufferSTL::FlushFiles %ld\n", size());
    // m_FileDataManager->FlushFiles(transportIndex);
}

} // end namespace adios2
