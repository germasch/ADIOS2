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
    if (m_DebugMode)
    {
        auto itVariable = m_Variables.find(name);
        if (itVariable != m_Variables.end())
        {
            throw std::invalid_argument("ERROR: variable " + name +
                                        " exists in IO object " + m_Name +
                                        ", in call to DefineVariable\n");
        }
    }

    auto &variableMap = GetVariableMap<T>();
    auto itVariable = variableMap.emplace(
        Variable<T>(name, shape, start, count, constantDims, m_DebugMode));
    typename VariableMap<T>::Index index = itVariable->first;
    Variable<T> &variable = itVariable->second;

    m_Variables.emplace(name, std::make_pair(helper::GetType<T>(), index));

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
    auto itVariable = m_Variables.find(name);

    if (itVariable == m_Variables.end())
    {
        return nullptr;
    }

    if (itVariable->second.first != helper::GetType<T>())
    {
        return nullptr;
    }

    Variable<T> *variable = &GetVariableMap<T>().at(itVariable->second.second);
    if (m_ReadStreaming)
    {
        if (!variable->IsValidStep(m_EngineStep + 1))
        {
            return nullptr;
        }
    }
    return variable;
}

template <class T>
Attribute<T> &IO::DefineAttributeCommon(const std::string &globalName,
                                        Attribute<T> &&attribute)
{
    if (m_DebugMode)
    {
        CheckAttributeCommon(globalName);
    }

    auto &attributeMap = GetAttributeMap<T>();
    auto itAttribute = attributeMap.emplace(std::move(attribute));
    typename AttributeMap<T>::Index index = itAttribute->first;

    m_Attributes.emplace(globalName,
                         std::make_pair(helper::GetType<T>(), index));

    return itAttribute->second;
}

template <class T>
Attribute<T> &IO::DefineAttribute(const std::string &name, const T &value,
                                  const std::string &variableName,
                                  const std::string separator)
{
    const std::string globalName =
        AttributeGlobalName(name, variableName, separator);

    return DefineAttributeCommon(globalName, Attribute<T>(globalName, value));
}

template <class T>
Attribute<T> &IO::DefineAttribute(const std::string &name, const T *array,
                                  const size_t elements,
                                  const std::string &variableName,
                                  const std::string separator)
{
    const std::string globalName =
        AttributeGlobalName(name, variableName, separator);

    return DefineAttributeCommon(globalName,
                                 Attribute<T>(globalName, array, elements));
}

template <class T>
Attribute<T> *IO::InquireAttribute(const std::string &name,
                                   const std::string &variableName,
                                   const std::string separator) noexcept
{
    const std::string globalName =
        AttributeGlobalName(name, variableName, separator);
    auto itAttribute = m_Attributes.find(globalName);

    if (itAttribute == m_Attributes.end())
    {
        return nullptr;
    }

    if (itAttribute->second.first != helper::GetType<T>())
    {
        return nullptr;
    }

    return &GetAttributeMap<T>().at(itAttribute->second.second);
}

// PRIVATE

// GetAttributeMap
#define make_GetAttributeMap(T, NAME)                                          \
    template <>                                                                \
    AttributeMap<T> &IO::GetAttributeMap() noexcept                            \
    {                                                                          \
        return m_##NAME##A;                                                    \
    }
ADIOS2_FOREACH_ATTRIBUTE_STDTYPE_2ARGS(make_GetAttributeMap)
#undef make_GetAttributeMap

} // end namespace core
} // end namespace adios2

#endif /* ADIOS2_CORE_IO_TCC_ */
