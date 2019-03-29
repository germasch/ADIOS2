/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * BufferSTL.h
 *
 *  Created on: Sep 26, 2017
 *      Author: William F Godoy godoywf@ornl.gov
 */

#ifndef ADIOS2_TOOLKIT_FORMAT_BUFFERSTL_H_
#define ADIOS2_TOOLKIT_FORMAT_BUFFERSTL_H_

#include <string>
#include <vector>

#include "adios2/ADIOSTypes.h"

namespace adios2
{

class BufferSTL
{
public:
    using iterator = std::vector<char>::iterator;
    using const_iterator = std::vector<char>::const_iterator;

    std::vector<char> m_Buffer;
    size_t m_Position = 0;
    size_t m_AbsolutePosition = 0;
    size_t m_AbsoluteOffset = 0;

    BufferSTL() = default;
    ~BufferSTL() = default;

    const char *data() const;
    char *data();
    size_t size() const;
    size_t capacity() const;

    const char &operator[](size_t index) const;
    char &operator[](size_t index);

    const_iterator begin() const;
    iterator begin();
    const_iterator end() const;
    iterator end();

    template <class InputIterator>
    iterator insert(iterator it, InputIterator first, InputIterator last);

    void assign(size_t count, const char &value);

    // resize() expects that new_size isn't larger than what's already reserved
    // If resize() expands the buffer, the newly added bytes are going to be
    // zero by means of the underlying buffer having been zero-initialized when
    // it was created or expanded by reserve()
    void resize(size_t new_size);
    void reserve(size_t new_capacity);

    void Resize(const size_t size, const std::string hint);
    void Reset(const bool resetAbsolutePosition, const bool zeroInitialize);

    size_t GetAvailableSize() const;

    size_t AbsolutePosition() const;
    void AbsolutePositionInc(size_t offset);

private:
    const bool m_DebugMode = false;
};

} // end namespace adios2

#include "BufferSTL.inl"

#endif /* ADIOS2_TOOLKIT_FORMAT_STLBUFFER_H_ */
