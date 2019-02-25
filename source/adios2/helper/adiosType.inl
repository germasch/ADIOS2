/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * adiosType.inl implementation of template functions in adiosType.h
 *
 *  Created on: May 17, 2017
 *      Author: William F Godoy godoywf@ornl.gov
 */

#ifndef ADIOS2_HELPER_ADIOSTYPE_INL_
#define ADIOS2_HELPER_ADIOSTYPE_INL_
#ifndef ADIOS2_HELPER_ADIOSTYPE_H_
#error "Inline file should only be included from it's header, never on it's own"
#endif

#include <algorithm> //std::transform
#include <sstream>   //std::ostringstream

#include "adios2/ADIOSMacros.h"

namespace adios2
{
namespace helper
{

template <class T, class U>
std::vector<U> NewVectorType(const std::vector<T> &in)
{
    return NewVectorTypeFromArray<T, U>(in.data(), in.size());
}

template <class T, class U>
std::vector<U> NewVectorTypeFromArray(const T *in, const size_t inSize)
{
    std::vector<U> out(inSize);
    std::transform(in, in + inSize, out.begin(),
                   [](T value) { return static_cast<U>(value); });
    return out;
}

template <class T>
constexpr bool IsLvalue(T &&)
{
    return std::is_lvalue_reference<T>{};
}

template <class T, class U>
U *InquireKey(const T &key, std::map<T, U> &input) noexcept
{
    auto itKey = input.find(key);
    if (itKey == input.end())
    {
        return nullptr;
    }
    return &itKey->second;
}

template <class T, class U>
U *InquireKey(const T &key, std::unordered_map<T, U> &input) noexcept
{
    auto itKey = input.find(key);
    if (itKey == input.end())
    {
        return nullptr;
    }
    return &itKey->second;
}

template <>
inline std::string ValueToString(const std::string value) noexcept
{
    return "\"" + value + "\"";
}

#define declare_template_instantiation(C)                                      \
    template <>                                                                \
    inline std::string ValueToString(const C value) noexcept                   \
    {                                                                          \
        const int valueInt = static_cast<int>(value);                          \
        return std::to_string(valueInt);                                       \
    }
ADIOS2_FOREACH_CHAR_TYPE_1ARG(declare_template_instantiation)
#undef declare_template_instantiation

template <class T>
inline std::string ValueToString(const T value) noexcept
{
    std::ostringstream valueSS;
    valueSS << value;
    const std::string valueStr(valueSS.str());
    return valueStr;
}

template <>
inline std::string VectorToCSV(const std::vector<std::string> &input) noexcept
{
    if (input.empty())
    {
        return std::string();
    }

    std::ostringstream valueSS;
    for (const auto value : input)
    {
        valueSS << "\"" << value << "\", ";
    }
    std::string csv(valueSS.str());
    csv.pop_back();
    csv.pop_back();

    return csv;
}

#define declare_template_instantiation(C)                                      \
    template <>                                                                \
    inline std::string VectorToCSV(const std::vector<C> &input) noexcept       \
    {                                                                          \
        if (input.empty())                                                     \
        {                                                                      \
            return std::string();                                              \
        }                                                                      \
                                                                               \
        std::ostringstream valueSS;                                            \
        for (const auto value : input)                                         \
        {                                                                      \
            const int valueInt = static_cast<int>(value);                      \
            valueSS << valueInt << ", ";                                       \
        }                                                                      \
        std::string csv(valueSS.str());                                        \
        csv.pop_back();                                                        \
        csv.pop_back();                                                        \
                                                                               \
        return csv;                                                            \
    }
ADIOS2_FOREACH_CHAR_TYPE_1ARG(declare_template_instantiation)
#undef declare_template_instantiation

template <class T>
inline std::string VectorToCSV(const std::vector<T> &input) noexcept
{
    if (input.empty())
    {
        return std::string();
    }

    std::ostringstream valueSS;
    for (const auto value : input)
    {
        valueSS << value << ", ";
    }
    std::string csv(valueSS.str());
    csv.pop_back();
    csv.pop_back();

    return csv;
}

template <class T>
void CheckForNullptr(T *pointer, const std::string hint)
{
    if (pointer == nullptr)
    {
        throw std::invalid_argument("ERROR: found null pointer " + hint + "\n");
    }
}

#define make_GetType(TYPE, NAME)                                               \
    template <>                                                                \
    inline DataType GetType<TYPE>()                                            \
    {                                                                          \
        return DataType::NAME;                                                 \
    }

ADIOS2_FOREACH_STDTYPE_2ARGS(make_GetType)
make_GetType(Compound, Compound);
make_GetType(Unknown, Unknown);
make_GetType(None, Unknown);
#undef make_GetType

} // end namespace helper
} // end namespace adios2

#endif /* ADIOS2_HELPER_ADIOSTYPE_INL_ */
