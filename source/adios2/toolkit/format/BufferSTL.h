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
#include "adios2/toolkit/transportman/TransportMan.h"

namespace adios2
{

class BackingStoreStdVector : std::vector<char>
{
    using Base = std::vector<char>;

public:
    using Base::data;
    using Base::size;
    using Base::begin;
    using Base::operator[];
    using Base::assign;
    using Base::reserve;
    using Base::resize;

    Base &Buffer() { return *this; }
    const Base &Buffer() const { return *this; }
};

class BufferSTL
{
    using BackingStore = BackingStoreStdVector;

public:
    using iterator = char *; // FIXME, should do real iterators?
    using const_iterator = const char *;

    size_t m_Position = 0;
    size_t m_AbsolutePosition = 0;

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

    void AbsoluteOffsetInc(size_t offset);
    void AbsoluteOffsetReset();

    const std::vector<char> &Buffer() const;
    std::vector<char> &Buffer();

    void WriteFiles(const int transportIndex);
    void FlushFiles(const int transportIndex);

    transportman::TransportMan *m_FileDataManager = nullptr; // FIXME
private:
    size_t m_AbsoluteOffset = 0;
    const bool m_DebugMode = false;
    BackingStore m_Buffer;
};

} // end namespace adios2

#include "BufferSTL.inl"

#endif /* ADIOS2_TOOLKIT_FORMAT_STLBUFFER_H_ */
