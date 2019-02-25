/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * ADIOSTypes.h : public header that contains "using/typedef" alias, defaults
 * and parameters options as enum classes
 *
 *  Created on: Mar 23, 2017
 *      Author: Chuck Atkins chuck.atkins@kitware.com
 *              Norbert Podhorszki pnorbert@ornl.gov
 *              William F Godoy godoywf@ornl.gov
 */

#ifndef ADIOS2_ADIOSTYPES_H_
#define ADIOS2_ADIOSTYPES_H_

/// \cond EXCLUDE_FROM_DOXYGEN
#include <cstddef>
#include <cstdint>

#include <complex>
#include <limits>
#include <map>
#include <string>
#include <type_traits>
#include <utility> //std::pair
#include <vector>
/// \endcond

#include "adios2/ADIOSConfig.h"

namespace adios2
{

/** Variable shape type identifier, assigned automatically from the signature of
 *  DefineVariable */
enum class ShapeID
{
    Unknown,     ///< undefined shapeID
    GlobalValue, ///< single global value, common case
    GlobalArray, ///< global (across MPI_Comm) array, common case
    JoinedArray, ///< global array with a common (joinable) dimension
    LocalValue,  ///< special case, local independent value
    LocalArray   ///< special case, local independent array
};

/** Used to set IO class */
enum class IOMode
{
    Independent, ///< all I/O operations are independent per rank
    Collective   ///< expect collective I/O operations
};

/** OpenMode in IO Open */
enum class Mode
{
    Undefined,
    // open modes
    Write,
    Read,
    Append,
    // launch execution modes
    Sync,
    Deferred
};

enum class ReadMultiplexPattern
{
    GlobalReaders,
    RoundRobin,
    FirstInFirstOut,
    OpenAllSteps
};

enum class StreamOpenMode
{
    Wait,
    NoWait
};

enum class ReadMode
{
    NonBlocking,
    Blocking
};

enum class StepMode
{
    Append,
    Update, // writer advance mode
    NextAvailable,
    LatestAvailable // reader advance mode
};

enum class StepStatus
{
    OK,
    NotReady,
    EndOfStream,
    OtherError
};

enum class TimeUnit
{
    Microseconds,
    Milliseconds,
    Seconds,
    Minutes,
    Hours
};

/** Type of selection */
enum class SelectionType
{
    BoundingBox, ///< Contiguous block of data defined by offsets and counts
                 /// per dimension
    Points,      ///< List of individual points
    WriteBlock,  ///< Selection of an individual block written by a writer
                 /// process
    Auto         ///< Let the engine decide what to return
};

class DataType
{
public:
    DataType() : m_Type() {}

    DataType &operator=(const DataType &other)
    {
        m_Type = other.m_Type;
        return *this;
    }

    bool operator==(const DataType &other) const
    {
        return m_Type == other.m_Type;
    }
    bool operator!=(const DataType &other) const { return !(*this == other); }

    const std::string &GetString() const { return m_Type; }

    static DataType Create(const std::string &s) { return DataType(s); }
private:
    DataType(const std::string &s) : m_Type(s) {}

    std::string m_Type;
};

// FIXME: This not pretty, but it's temporary
struct Compound;
struct Unknown;
struct None;

// Types
using std::size_t;

using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

// Complex
using cfloat = std::complex<float>;
using cdouble = std::complex<double>;

// Limit, using uint64_t to make it portable
constexpr uint64_t MaxU64 = std::numeric_limits<uint64_t>::max();
constexpr size_t MaxSizeT = std::numeric_limits<size_t>::max();
constexpr size_t DefaultSizeT = std::numeric_limits<size_t>::max();
constexpr size_t EngineCurrentStep = std::numeric_limits<size_t>::max();

// adios defaults
#ifdef _WIN32
const std::string DefaultFileLibrary("fstream");
#else
const std::string DefaultFileLibrary("POSIX");
#endif
const std::string DefaultTimeUnit("Microseconds");
constexpr TimeUnit DefaultTimeUnitEnum(TimeUnit::Microseconds);

/** default initial bp buffer size, 16Kb, in bytes */
constexpr size_t DefaultInitialBufferSize = 16 * 1024;

/** default maximum bp buffer size, unlimited, in bytes.
 *  Needs to be studied for optimizing applications */
constexpr uint64_t DefaultMaxBufferSize = MaxSizeT - 1;

/** default buffer growth factor. Needs to be studied
 * for optimizing applications*/
constexpr float DefaultBufferGrowthFactor = 1.05f;

/** default size for writing/reading files using POSIX/fstream/stdio write
 *  2Gb - 100Kb (tolerance)*/
constexpr size_t DefaultMaxFileBatchSize = 2147381248;

constexpr char PathSeparator =
#ifdef _WIN32
    '\\';
#else
    '/';
#endif

// adios alias values and types
constexpr bool DebugON = true;
constexpr bool DebugOFF = false;
constexpr size_t UnknownDim = 0;
constexpr uint64_t JoinedDim = MaxU64 - 1;
constexpr uint64_t LocalValueDim = MaxU64 - 2;
constexpr uint64_t IrregularDim = MaxU64 - 3;
constexpr bool ConstantDims = true;
constexpr bool endl = true;

using Dims = std::vector<size_t>;
using Params = std::map<std::string, std::string>;
using vParams = std::vector<std::pair<std::string, Params>>;
using Steps = size_t;

template <class T>
using Box = std::pair<T, T>;

/**
 * TypeInfo
 * used to map from primitive types to stdint-based types
 */
template <typename T, typename Enable = void>
struct TypeInfo;

/**
 * ToString
 * makes a string from an enum class like ShapeID etc, for debugging etc
 */

std::string ToString(ShapeID value);
std::string ToString(IOMode value);
std::string ToString(Mode value);
std::string ToString(DataType value);

/**
 * operator<<(ostream&, T enum_val)
 * enables output of enum classes directly to std::cout etc,
 * if ToString() can handle it
 */
template <typename T, typename Enable = decltype(ToString(std::declval<T>()))>
std::ostream &operator<<(std::ostream &os, const T &value);

// FIXME, temp workaround for ambiguous operator<< templates
inline std::ostream &operator<<(std::ostream &os, const char *value)
{
    return std::operator<<(os, value);
}

inline std::ostream &operator<<(std::ostream &os, const std::string &value)
{
    return std::operator<<(os, value);
}

} // end namespace adios2

#include "ADIOSTypes.inl"

#endif /* ADIOS2_ADIOSTYPES_H_ */
