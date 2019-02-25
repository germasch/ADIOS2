/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * Attribute.tcc :
 *
 *  Created on: Jun 4, 2018
 *      Author: William F Godoy godoywf@ornl.gov
 */

#ifndef ADIOS2_BINDINGS_CXX11_CXX11_ATTRIBUTE_TCC_
#define ADIOS2_BINDINGS_CXX11_CXX11_ATTRIBUTE_TCC_

#include "Attribute.h"

#include "adios2/helper/adiosFunctions.h"

namespace adios2
{

template <class T>
Attribute<T>::Attribute(core::Attribute<IOType> *attribute)
: m_Attribute(attribute)
{
}

template <class T>
Attribute<T>::operator bool() const noexcept
{
    return (m_Attribute == nullptr) ? false : true;
}

template <class T>
std::string Attribute<T>::Name() const
{
    helper::CheckForNullptr(m_Attribute, "in call to Attribute<T>::Name()");
    return m_Attribute->m_Name;
}

template <class T>
std::string Attribute<T>::Type() const
{
    helper::CheckForNullptr(m_Attribute, "in call to Attribute<T>::Type()");
    return ToString(m_Attribute->m_Type);
}

template <class T>
std::vector<T> Attribute<T>::Data() const
{
    helper::CheckForNullptr(m_Attribute, "in call to Attribute<T>::Data()");

    if (m_Attribute->m_IsSingleValue)
    {
        return std::vector<T>{m_Attribute->m_DataSingleValue};
    }
    else
    {
        return reinterpret_cast<std::vector<T> &>(m_Attribute->m_DataArray);
    }
}

} // end namespace adios2

#endif /* ADIOS2_BINDINGS_CXX11_CXX11_ATTRIBUTE_TCC_ */
