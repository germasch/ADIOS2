/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * ADIOSMacros.h : a set of helper macros used internally
 *
 * Created on: March 23, 2017
 *     Author: Chuck Atkins chuck.atkins@kitware.com
 */

#ifndef ADIOS2_ADIOSMACROS_H
#define ADIOS2_ADIOSMACROS_H

#include <string>

#include "adios2/ADIOSTypes.h"

/**
 <pre>
 The ADIOS_FOREACH_STTYPE_1ARG macro assumes the given argument is a macro which
 takes a single argument that is a type and then inserts the given MACRO for
 each of the known primitive types

 An example of this might be to instantiate a template function for each
 known type.  For example:

   template<typename T> int foo() { // some implementation of foo  }

   #define instantiate_foo(T) template int foo<T>();
   ADIOS_FOREACH_STDTYPE_1ARG(instantiate_foo)
   #undef instantiate_foo
 </pre>
*/
#define ADIOS2_FOREACH_ATTRIBUTE_PRIMITIVE_STDTYPE_1ARG(MACRO)                 \
    MACRO(int8_t)                                                              \
    MACRO(int16_t)                                                             \
    MACRO(int32_t)                                                             \
    MACRO(int64_t)                                                             \
    MACRO(uint8_t)                                                             \
    MACRO(uint16_t)                                                            \
    MACRO(uint32_t)                                                            \
    MACRO(uint64_t)                                                            \
    MACRO(float)                                                               \
    MACRO(double)                                                              \
    MACRO(long double)

#define ADIOS2_FOREACH_PRIMITIVE_STDTYPE_1ARG(MACRO)                           \
    ADIOS2_FOREACH_ATTRIBUTE_PRIMITIVE_STDTYPE_1ARG(MACRO)                     \
    MACRO(std::complex<float>)                                                 \
    MACRO(std::complex<double>)

#define ADIOS2_FOREACH_ATTRIBUTE_STDTYPE_1ARG(MACRO)                           \
    MACRO(std::string)                                                         \
    ADIOS2_FOREACH_ATTRIBUTE_PRIMITIVE_STDTYPE_1ARG(MACRO)

#define ADIOS2_FOREACH_STDTYPE_1ARG(MACRO)                                     \
    MACRO(std::string)                                                         \
    ADIOS2_FOREACH_PRIMITIVE_STDTYPE_1ARG(MACRO)

/**
 * The ADIOS_FOREACH_TYPE_1ARG macro is similar, but uses primitive C++ types
 * rather than stdint based types. Should only be used in CXX11 bindings, not in
 * the core.
 */
#define ADIOS2_FOREACH_ATTRIBUTE_PRIMITIVE_TYPE_1ARG(MACRO)                    \
    MACRO(char)                                                                \
    MACRO(signed char)                                                         \
    MACRO(unsigned char)                                                       \
    MACRO(short)                                                               \
    MACRO(unsigned short)                                                      \
    MACRO(int)                                                                 \
    MACRO(unsigned int)                                                        \
    MACRO(long int)                                                            \
    MACRO(unsigned long int)                                                   \
    MACRO(long long int)                                                       \
    MACRO(unsigned long long int)                                              \
    MACRO(float)                                                               \
    MACRO(double)                                                              \
    MACRO(long double)

#define ADIOS2_FOREACH_PRIMITIVE_TYPE_1ARG(MACRO)                              \
    ADIOS2_FOREACH_ATTRIBUTE_PRIMITIVE_TYPE_1ARG(MACRO)                        \
    MACRO(std::complex<float>)                                                 \
    MACRO(std::complex<double>)

#define ADIOS2_FOREACH_ATTRIBUTE_TYPE_1ARG(MACRO)                              \
    MACRO(std::string)                                                         \
    ADIOS2_FOREACH_ATTRIBUTE_PRIMITIVE_TYPE_1ARG(MACRO)

#define ADIOS2_FOREACH_TYPE_1ARG(MACRO)                                        \
    MACRO(std::string)                                                         \
    ADIOS2_FOREACH_PRIMITIVE_TYPE_1ARG(MACRO)

#define ADIOS2_FOREACH_COMPLEX_PRIMITIVE_TYPE_1ARG(MACRO)                      \
    MACRO(float)                                                               \
    MACRO(double)                                                              \
    MACRO(long double)

#define ADIOS2_FOREACH_CHAR_TYPE_1ARG(MACRO)                                   \
    MACRO(char)                                                                \
    MACRO(signed char)                                                         \
    MACRO(unsigned char)

#define ADIOS2_FOREACH_ZFP_TYPE_1ARG(MACRO)                                    \
    MACRO(int32_t)                                                             \
    MACRO(int64_t)                                                             \
    MACRO(float)                                                               \
    MACRO(double)

#define ADIOS2_FOREACH_SZ_TYPE_1ARG(MACRO)                                     \
    MACRO(float)                                                               \
    MACRO(double)

#define ADIOS2_FOREACH_MGARD_TYPE_1ARG(MACRO) MACRO(double)

/**
 <pre>
 The ADIOS2_FOREACH_STDTYPE_2ARGS macro assumes the given argument is a macro
 which takes two arguments, the first being a type, and the second being a
 label for that type, i.e. std::complex<float> and CFloat.

 An example of this might be to define a virtual method for each known
 type, and since the method is virtual then it cannot be a template.
 For example:

   #define declare_foo(T,L) virtual const T& foo ## L (std::string bar);
   ADIOS2_FOREACH_STDTYPE_2ARGS(declare_foo)
   #undef declare_foo

   is equivalent to:

   virtual           char& foo_Int8(std::string bar);
   virtual unsigned  char& foo_UInt8(std::string bar);
   virtual          short& foo_Int16(std::string bar);
   virtual unsigned short& foo_UInt16(std::string bar);
   ...
   virtual std::complex<double>& foo_CDouble(std::string bar);
  </pre>
*/
#define ADIOS2_FOREACH_ATTRIBUTE_STDTYPE_2ARGS(MACRO)                          \
    MACRO(std::string, String)                                                 \
    MACRO(int8_t, Int8)                                                        \
    MACRO(uint8_t, UInt8)                                                      \
    MACRO(int16_t, Int16)                                                      \
    MACRO(uint16_t, UInt16)                                                    \
    MACRO(int32_t, Int32)                                                      \
    MACRO(uint32_t, UInt32)                                                    \
    MACRO(int64_t, Int64)                                                      \
    MACRO(uint64_t, UInt64)                                                    \
    MACRO(float, Float)                                                        \
    MACRO(double, Double)                                                      \
    MACRO(long double, LDouble)

#define ADIOS2_FOREACH_STDTYPE_2ARGS(MACRO)                                    \
    ADIOS2_FOREACH_ATTRIBUTE_STDTYPE_2ARGS(MACRO)                              \
    MACRO(std::complex<float>, CFloat)                                         \
    MACRO(std::complex<double>, CDouble)

#define ADIOS2_FOREACH_ATTRIBUTE_STDTYPE_4ARGS(MACRO, IO, FUNC, ARGS...)       \
    MACRO(std::string, IO, FUNC, ARGS)                                         \
    MACRO(int8_t, IO, FUNC, ARGS)                                              \
    MACRO(int16_t, IO, FUNC, ARGS)                                             \
    MACRO(int32_t, IO, FUNC, ARGS)                                             \
    MACRO(int64_t, IO, FUNC, ARGS)                                             \
    MACRO(uint8_t, IO, FUNC, ARGS)                                             \
    MACRO(uint16_t, IO, FUNC, ARGS)                                            \
    MACRO(uint32_t, IO, FUNC, ARGS)                                            \
    MACRO(uint64_t, IO, FUNC, ARGS)                                            \
    MACRO(float, IO, FUNC, ARGS)                                               \
    MACRO(double, IO, FUNC, ARGS)                                              \
    MACRO(long double, IO, FUNC, ARGS)

#define MAKE_IO_FOREACH_ATTRIBUTE_CASE(TYPE, IO, FUNC, ARGS...)                \
    else if (attrType == helper::GetType<TYPE>())                              \
    {                                                                          \
        auto &attribute = *IO.InquireAttribute<TYPE>(attrName);                \
        FUNC(attribute, ARGS);                                                 \
    }

#define ADIOS2_IO_FOREACH_ATTRIBUTE(IO, FUNC, ARGS...)                         \
    for (const auto &apair : io.GetAttributesDataMap())                        \
    {                                                                          \
        std::string attrName = apair.first;                                    \
        DataType attrType = apair.second.first;                                \
        unsigned int index = apair.second.second;                              \
                                                                               \
        if (false)                                                             \
        {                                                                      \
        }                                                                      \
        ADIOS2_FOREACH_ATTRIBUTE_STDTYPE_4ARGS(MAKE_IO_FOREACH_ATTRIBUTE_CASE, \
                                               IO, FUNC, ARGS)                 \
    }

#endif /* ADIOS2_ADIOSMACROS_H */
