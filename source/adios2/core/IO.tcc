/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * IO.tcc template implementations with fix types and specializations
 *
 *  Created on: May 15, 2017
 *      Author: William F Godoy godoywf@ornl.gov
 */

#ifndef ADIOS2_CORE_IO_TCC_
#define ADIOS2_CORE_IO_TCC_

#include "IO.h"

/// \cond EXCLUDE_FROM_DOXYGEN
#include <iostream>
#include <stdexcept> //std::invalid_argument
/// \endcond

#include "adios2/ADIOSMPI.h"
#include "adios2/ADIOSMacros.h"
#include "adios2/helper/adiosFunctions.h" //helper::GetType<T>

namespace adios2
{
namespace core
{

template <class T>
Variable<T> &IO::DefineVariable(const std::string &name, const Dims &shape,
                                const Dims &start, const Dims &count,
                                const bool constantDims)
{
    Variable<T> &variable = m_Variables.emplace<T>(name, shape, start, count,
                                                   constantDims, m_DebugMode);

    // check IO placeholder for variable operations
    auto itOperations = m_VarOpsPlaceholder.find(name);
    if (itOperations != m_VarOpsPlaceholder.end())
    {
        variable.m_Operations.reserve(itOperations->second.size());

        for (auto &operation : itOperations->second)
        {
            variable.AddOperation(*operation.Op, operation.Parameters);
        }
    }

    return variable;
}

template <class T>
Variable<T> *IO::InquireVariable(const std::string &name) noexcept
{
    auto range = m_Variables.range();
    auto it = range.find(name);
    if (it == range.end() || it->m_Type != helper::GetType<T>())
    {
        return nullptr;
    }

    auto var = it.wrapper();
    auto &variable = var.Get<T>();
    if (m_ReadStreaming)
    {
        if (!variable.IsValidStep(m_EngineStep + 1))
        {
            return nullptr;
        }
    }
    return &variable;
}

template <class T>
Attribute<T> &IO::DefineAttribute(const std::string &name, const T &value,
                                  const std::string &variableName,
                                  const std::string separator)
{
    const std::string globalName =
        AttributeGlobalName(name, variableName, separator);

    return m_Attributes.emplace<T>(globalName, value);
}

template <class T>
Attribute<T> &IO::DefineAttribute(const std::string &name, const T *array,
                                  const size_t elements,
                                  const std::string &variableName,
                                  const std::string separator)
{
    const std::string globalName =
        AttributeGlobalName(name, variableName, separator);

    return m_Attributes.emplace<T>(globalName, array, elements);
}

template <class T>
Attribute<T> *IO::InquireAttribute(const std::string &name,
                                   const std::string &variableName,
                                   const std::string separator) noexcept
{
    const std::string globalName =
        AttributeGlobalName(name, variableName, separator);

    auto range = m_Attributes.range();
    auto it = range.find(globalName);
    if (it == range.end() || it->m_Type != helper::GetType<T>())
    {
        return nullptr;
    }

    auto attr = it.wrapper();
    auto &attribute = attr.Get<T>();
    return &attribute;
}

} // end namespace core
} // end namespace adios2

#endif /* ADIOS2_CORE_IO_TCC_ */
