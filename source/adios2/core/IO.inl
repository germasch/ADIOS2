/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * IO.inl template inline implementations
 *
 *  Created on: Jan 25, 2019
 *      Author: Kai Germaschewski <kai.germaschewski@unh.edu>
 */

#ifndef ADIOS2_CORE_IO_INL_
#define ADIOS2_CORE_IO_INL_
#ifndef ADIOS2_CORE_IO_H_
#error "Inline file should only be included from its header, never on its own"
#endif

namespace adios2
{
namespace core
{

template <template <class> class Entity, class T>
inline void EntityMap<Entity, T>::erase(Index key)
{
    m_Map.erase(key);
}

template <template <class> class Entity, class T>
inline void EntityMap<Entity, T>::clear() noexcept
{
    m_Map.clear();
}

template <template <class> class Entity, class T>
inline typename EntityMap<Entity, T>::Value &EntityMap<Entity, T>::at(Index key)
{
    return m_Map.at(key);
}

template <template <class> class Entity, class T>
inline const typename EntityMap<Entity, T>::Value &
EntityMap<Entity, T>::at(Index key) const
{
    return m_Map.at(key);
}

template <template <class> class Entity, class T>
template <class... Args>
inline typename EntityMap<Entity, T>::iterator
EntityMap<Entity, T>::emplace(Args &&... args)
{
    auto status = m_Map.emplace(std::piecewise_construct,
                                std::forward_as_tuple(m_Index++),
                                std::forward_as_tuple(args...));
    if (!status.second)
    {
        throw std::runtime_error("emplace failed in EntityMap::emplace");
    }
    return status.first;
}

} // namespace core
} // namespace adios2

#endif /* ADIOS2_CORE_IO_INL_ */
