/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * BufferSTL.h
 *
 *  Created on: Mar 26, 2019
 *      Author: Kai Germaschewski <kai.germaschewski@unh.edu>
 */

#ifndef ADIOS2_TOOLKIT_FORMAT_BUFFERSTL_INL_
#define ADIOS2_TOOLKIT_FORMAT_BUFFERSTL_INL_
#ifndef ADIOS2_TOOLKIT_FORMAT_BUFFERSTL_H_
#error "Inline file should only be included from its header, never on its own"
#endif

#include <cassert>

namespace adios2
{

inline const char *BufferSTL::data() const { return m_Buffer.data(); }
inline char *BufferSTL::data() { return m_Buffer.data(); }
inline size_t BufferSTL::size() const { return m_Position; }
inline size_t BufferSTL::capacity() const { return m_Buffer.size(); }

inline BufferSTL::const_iterator BufferSTL::begin() const
{
    return m_Buffer.begin();
}
inline BufferSTL::iterator BufferSTL::begin() { return m_Buffer.begin(); }
inline BufferSTL::const_iterator BufferSTL::end() const
{
    return m_Buffer.begin() + m_Position;
}
inline BufferSTL::iterator BufferSTL::end()
{
    return m_Buffer.begin() + m_Position;
}

inline const char &BufferSTL::operator[](size_t index) const
{
    return m_Buffer[index];
}

inline char &BufferSTL::operator[](size_t index) { return m_Buffer[index]; }

template <class InputIterator>
inline BufferSTL::iterator BufferSTL::insert(iterator it, InputIterator first,
                                             InputIterator last)
{
    // for now, this only for use in helper::InsertToBuffer,
    // which always appends.
    assert(it == end());
    std::copy(first, last, it);
    m_Position += (last - first);
    return end();
}

inline void BufferSTL::assign(size_t count, const char &value)
{
    m_Buffer.assign(count, value);
}

inline void BufferSTL::resize(size_t new_size) { m_Position = new_size; }

inline void BufferSTL::reserve(size_t new_capacity)
{
    Resize(new_capacity, "in BufferSTL::reserve");
}

} // end namespace adios2

#endif /* ADIOS2_TOOLKIT_FORMAT_STLBUFFER_INL_ */
