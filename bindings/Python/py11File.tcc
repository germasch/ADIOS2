/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * py11File.tcc
 *
 *  Created on: May 29, 2019
 *      Author: William F Godoy godoywf@ornl.gov
 */

#ifndef ADIOS2_BINDINGS_PYTHON_PY11FILE_TCC_
#define ADIOS2_BINDINGS_PYTHON_PY11FILE_TCC_

#include "py11File.h"

namespace adios2
{
namespace py11
{

template <class T>
pybind11::array File::DoRead(core::Variable<T> &variable, const size_t blockID)
{
    Dims start;
    Dims count;

    if (variable.m_ShapeID == ShapeID::GlobalArray)
    {
        count = variable.Shape();
        start = Dims(count.size(), 0);
        return Read(variable.m_Name, start, count, blockID);
    }
    else if (variable.m_ShapeID == ShapeID::LocalArray)
    {
        variable.SetBlockSelection(blockID);
        count = variable.Count();
        start = Dims(count.size(), 0);
        return Read(variable.m_Name, start, count, blockID);
    }
    else
    {
        if (variable.m_SingleValue)
        {
            count = Dims{1};
            pybind11::array_t<T> pyArray(count);
            m_Stream->Read<T>(variable.m_Name, pyArray.mutable_data(), blockID);
            return pyArray;
        }
    }

    return pybind11::array();
}

template <class T>
pybind11::array File::DoRead(const std::string &name, const Dims &start,
                             const Dims &count, const size_t blockID)
{
    pybind11::array_t<T> pyArray(count);
    m_Stream->Read<T>(name, pyArray.mutable_data(), Box<Dims>(start, count),
                      blockID);
    return pyArray;
}

} // end namespace py11
} // end namespace adios2

#endif
