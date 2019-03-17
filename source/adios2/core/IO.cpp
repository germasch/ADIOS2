/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * IO.cpp
 *
 *  Created on: Jan 6, 2017
 *      Author: William F Godoy godoywf@ornl.gov
 */

#include "IO.h"
#include "IO.tcc"

#include <sstream>

#include "adios2/ADIOSMPI.h"
#include "adios2/ADIOSMacros.h"

#include "adios2/engine/bp3/BP3Reader.h"
#include "adios2/engine/bp3/BP3Writer.h"
#include "adios2/engine/inline/InlineReader.h"
#include "adios2/engine/inline/InlineWriter.h"

/*BP4 engine headers*/
#include "adios2/engine/bp4/BP4Reader.h"
#include "adios2/engine/bp4/BP4Writer.h"

#include "adios2/engine/skeleton/SkeletonReader.h"
#include "adios2/engine/skeleton/SkeletonWriter.h"
#include "adios2/helper/adiosFunctions.h" //BuildParametersMap

#ifdef ADIOS2_HAVE_DATAMAN // external dependencies
#include "adios2/engine/dataman/DataManReader.h"
#include "adios2/engine/dataman/DataManWriter.h"
#endif

#ifdef ADIOS2_HAVE_WDM // external dependencies
#include "adios2/engine/wdm/WdmReader.h"
#include "adios2/engine/wdm/WdmWriter.h"
#endif

#ifdef ADIOS2_HAVE_SST // external dependencies
#include "adios2/engine/sst/SstReader.h"
#include "adios2/engine/sst/SstWriter.h"
#endif

#ifdef ADIOS2_HAVE_HDF5 // external dependencies
#include "adios2/engine/hdf5/HDF5ReaderP.h"
#include "adios2/engine/hdf5/HDF5WriterP.h"
#if H5_VERSION_GE(1, 11, 0)
#include "adios2/engine/mixer/HDFMixer.h"
#endif
#endif

#ifdef ADIOS2_HAVE_MPI // external dependencies
#include "adios2/engine/insitumpi/InSituMPIReader.h"
#include "adios2/engine/insitumpi/InSituMPIWriter.h"
#endif

namespace adios2
{
namespace core
{

IO::IO(ADIOS &adios, const std::string name, MPI_Comm mpiComm,
       const bool inConfigFile, const std::string hostLanguage,
       const bool debugMode)
: m_ADIOS(adios), m_Name(name), m_MPIComm(mpiComm),
  m_InConfigFile(inConfigFile), m_HostLanguage(hostLanguage),
  m_DebugMode(debugMode)
{
}

void IO::SetEngine(const std::string engineType) noexcept
{
    m_EngineType = engineType;
}
void IO::SetIOMode(const IOMode ioMode) { m_IOMode = ioMode; };

void IO::SetParameters(const Params &parameters) noexcept
{
    m_Parameters = parameters;
}

void IO::SetParameter(const std::string key, const std::string value) noexcept
{
    m_Parameters[key] = value;
}

Params &IO::GetParameters() noexcept { return m_Parameters; }

size_t IO::AddTransport(const std::string type, const Params &parameters)
{
    Params parametersMap(parameters);
    if (m_DebugMode)
    {
        if (parameters.count("transport") == 1 ||
            parameters.count("Transport") == 1)
        {
            throw std::invalid_argument("ERROR: key Transport (or transport) "
                                        "is not valid for transport type " +
                                        type + ", in call to AddTransport)");
        }

        CheckTransportType(type);
    }

    parametersMap["transport"] = type;
    m_TransportsParameters.push_back(parametersMap);
    return m_TransportsParameters.size() - 1;
}

void IO::SetTransportParameter(const size_t transportIndex,
                               const std::string key, const std::string value)
{
    if (m_DebugMode)
    {
        if (transportIndex >= m_TransportsParameters.size())
        {
            throw std::invalid_argument(
                "ERROR: transportIndex is larger than "
                "transports created with AddTransport, for key: " +
                key + ", value: " + value +
                "in call to SetTransportParameter\n");
        }
    }

    m_TransportsParameters[transportIndex][key] = value;
}

const DataMap<Variable> &IO::GetVariablesDataMap() const noexcept
{
    return m_Variables;
}

const DataMap<Attribute> &IO::GetAttributesDataMap() const noexcept
{
    return m_Attributes;
}

bool IO::InConfigFile() const noexcept { return m_InConfigFile; };

void IO::SetDeclared() noexcept { m_IsDeclared = true; };

bool IO::IsDeclared() const noexcept { return m_IsDeclared; }

bool IO::RemoveVariable(const std::string &name) noexcept
{
    return m_Variables.Remove(name);
}

void IO::RemoveAllVariables() noexcept
{
    m_Variables.RemoveAll();
}

bool IO::RemoveAttribute(const std::string &name) noexcept
{
    return m_Attributes.Remove(name);
}

void IO::RemoveAllAttributes() noexcept
{
    m_Attributes.RemoveAll();
}

struct IO::AddAvailableVariable
{
    template <class T>
    void operator()(const Variable<T> &variable,
                    std::map<std::string, Params> &info) noexcept
    {
        const std::string &name = variable.m_Name;
        info[name]["Type"] = ToString(variable.m_Type);
        info[name]["AvailableStepsCount"] =
            helper::ValueToString(variable.m_AvailableStepsCount);
        info[name]["Shape"] = helper::VectorToCSV(variable.m_Shape);
        if (variable.m_SingleValue)
        {
            info[name]["SingleValue"] = "true";
            info[name]["Value"] = helper::ValueToString(variable.m_Value);
        }
        else
        {
            info[name]["SingleValue"] = "false";
            info[name]["Min"] = helper::ValueToString(variable.m_Min);
            info[name]["Max"] = helper::ValueToString(variable.m_Max);
        }
    }
};

std::map<std::string, Params> IO::GetAvailableVariables() noexcept
{
    std::map<std::string, Params> variablesInfo;
    for (auto &var : m_Variables.range())
    {
        var.Visit(AddAvailableVariable(), variablesInfo);
    }
    return variablesInfo;
}

struct IO::AddAvailableAttribute
{
    template <class T>
    void operator()(const Attribute<T> &attribute,
                    const std::string &variablePrefix,
                    std::map<std::string, Params> &info) noexcept
    {
        std::string name = attribute.m_Name;
        if (!variablePrefix.empty())
        {
            // valid associated attribute
            if (name.size() <= variablePrefix.size())
            {
                return;
            }

            if (name.compare(0, variablePrefix.size(), variablePrefix) == 0)
            {
                name = name.substr(variablePrefix.size());
            }
            else
            {
                return;
            }
        }

        info[name]["Type"] = ToString(attribute.m_Type);

        info[name]["Elements"] = std::to_string(attribute.m_Elements);

        if (attribute.m_IsSingleValue)
        {
            info[name]["Value"] =
                helper::ValueToString(attribute.m_DataSingleValue);
        }
        else
        {
            info[name]["Value"] =
                "{ " + helper::VectorToCSV(attribute.m_DataArray) + " }";
        }
    }
};

std::map<std::string, Params>
IO::GetAvailableAttributes(const std::string &variableName,
                           const std::string separator) noexcept
{
    std::map<std::string, Params> attributesInfo;
    std::string variablePrefix;
    if (!variableName.empty())
    {
        variablePrefix = variableName + separator;
    }

    for (auto &attr : m_Attributes.range())
    {
        attr.Visit(AddAvailableAttribute(), variablePrefix, attributesInfo);
    }
    return attributesInfo;
}

struct IsValidStep // FIXME IO::
{
    template <typename T>
    bool operator()(const Variable<T> &variable, int step)
    {
        return variable.IsValidStep(step);
    }
};

DataType IO::InquireVariableType(const std::string &name) const noexcept
{
    auto variables = m_Variables.range();
    auto it = variables.find(name);
    if (it == variables.end())
    {
        return DataType::Unknown;
    }

    DataType type = it->m_Type;

    if (m_ReadStreaming)
    {
        if (!it->Visit(IsValidStep(), m_EngineStep + 1))
        {
            type = DataType::Unknown;
        }
    }

    return type;
}

DataType IO::InquireAttributeType(const std::string &name,
                                  const std::string &variableName,
                                  const std::string separator) const noexcept
{
    auto attributes = m_Attributes.range();
    const std::string globalName =
        AttributeGlobalName(name, variableName, separator);

    auto it = attributes.find(globalName);
    if (it == attributes.end())
    {
        return DataType::Unknown;
    }

    return it->m_Type;
}

size_t IO::AddOperation(Operator &op, const Params &parameters) noexcept
{
    m_Operations.push_back(Operation{&op, parameters, Params()});
    return m_Operations.size() - 1;
}

Engine &IO::Open(const std::string &name, const Mode mode,
                 MPI_Comm mpiComm_orig)
{
    auto itEngineFound = m_Engines.find(name);
    const bool isEngineFound = (itEngineFound != m_Engines.end());
    bool isEngineActive = false;
    if (isEngineFound)
    {
        if (*itEngineFound->second)
        {
            isEngineActive = true;
        }
    }

    if (m_DebugMode && isEngineFound)
    {
        if (isEngineActive) // check if active
        {
            throw std::invalid_argument("ERROR: IO Engine with name " + name +
                                        " already created and is active (Close "
                                        "not called yet), in call to Open.\n");
        }
    }

    if (isEngineFound)
    {
        if (!isEngineActive)
        {
            m_Engines.erase(name);
        }
    }

    MPI_Comm mpiComm;
    MPI_Comm_dup(mpiComm_orig, &mpiComm);
    std::shared_ptr<Engine> engine;
    const bool isDefaultEngine = m_EngineType.empty() ? true : false;
    std::string engineTypeLC = m_EngineType;
    if (!isDefaultEngine)
    {
        std::transform(engineTypeLC.begin(), engineTypeLC.end(),
                       engineTypeLC.begin(), ::tolower);
    }

    if (isDefaultEngine || engineTypeLC == "bpfile" || engineTypeLC == "bp3" ||
        engineTypeLC == "bp")
    {
        if (mode == Mode::Read)
        {
            engine =
                std::make_shared<engine::BP3Reader>(*this, name, mode, mpiComm);
        }
        else
        {
            engine =
                std::make_shared<engine::BP3Writer>(*this, name, mode, mpiComm);
        }

        m_EngineType = "bp";
    }
    else if (engineTypeLC == "bp4" || engineTypeLC == "bp4file")
    {
        if (mode == Mode::Read)
        {
            engine =
                std::make_shared<engine::BP4Reader>(*this, name, mode, mpiComm);
        }
        else
        {
            engine =
                std::make_shared<engine::BP4Writer>(*this, name, mode, mpiComm);
        }

        m_EngineType = "bp4file";
    }
    else if (engineTypeLC == "hdfmixer")
    {
#ifdef ADIOS2_HAVE_HDF5
#if H5_VERSION_GE(1, 11, 0)
        if (mode == Mode::Read)
            engine = std::make_shared<engine::HDF5ReaderP>(*this, name, mode,
                                                           mpiComm);
        else
            engine =
                std::make_shared<engine::HDFMixer>(*this, name, mode, mpiComm);
#else
        throw std::invalid_argument(
            "ERROR: update HDF5 >= 1.11 to support VDS.");
#endif
#else
        throw std::invalid_argument("ERROR: this version didn't compile with "
                                    "HDF5 library, can't use HDF5 engine\n");
#endif
    }
    else if (engineTypeLC == "dataman")
    {
#ifdef ADIOS2_HAVE_DATAMAN
        if (mode == Mode::Read)
            engine = std::make_shared<engine::DataManReader>(*this, name, mode,
                                                             mpiComm);
        else
            engine = std::make_shared<engine::DataManWriter>(*this, name, mode,
                                                             mpiComm);
#else
        throw std::invalid_argument(
            "ERROR: this version didn't compile with "
            "DataMan library, can't use DataMan engine\n");
#endif
    }
    else if (engineTypeLC == "wdm")
    {
#ifdef ADIOS2_HAVE_WDM
        if (mode == Mode::Read)
            engine =
                std::make_shared<engine::WdmReader>(*this, name, mode, mpiComm);
        else
            engine =
                std::make_shared<engine::WdmWriter>(*this, name, mode, mpiComm);
#else
        throw std::invalid_argument(
            "ERROR: this version didn't compile with "
            "DataMan library, can't use DataMan engine\n");
#endif
    }
    else if (engineTypeLC == "sst" || engineTypeLC == "effis")
    {
#ifdef ADIOS2_HAVE_SST
        if (mode == Mode::Read)
            engine =
                std::make_shared<engine::SstReader>(*this, name, mode, mpiComm);
        else
            engine =
                std::make_shared<engine::SstWriter>(*this, name, mode, mpiComm);
#else
        throw std::invalid_argument("ERROR: this version didn't compile with "
                                    "Sst library, can't use Sst engine\n");
#endif
    }
    else if (engineTypeLC == "hdf5")
    {
#ifdef ADIOS2_HAVE_HDF5
        if (mode == Mode::Read)
            engine = std::make_shared<engine::HDF5ReaderP>(*this, name, mode,
                                                           mpiComm);
        else
            engine = std::make_shared<engine::HDF5WriterP>(*this, name, mode,
                                                           mpiComm);
#else
        throw std::invalid_argument("ERROR: this version didn't compile with "
                                    "HDF5 library, can't use HDF5 engine\n");
#endif
    }
    else if (engineTypeLC == "insitumpi")
    {
#ifdef ADIOS2_HAVE_MPI
        if (mode == Mode::Read)
            engine = std::make_shared<engine::InSituMPIReader>(*this, name,
                                                               mode, mpiComm);
        else
            engine = std::make_shared<engine::InSituMPIWriter>(*this, name,
                                                               mode, mpiComm);
#else
        throw std::invalid_argument("ERROR: this version didn't compile with "
                                    "MPI, can't use InSituMPI engine\n");
#endif
    }
    else if (engineTypeLC == "skeleton")
    {
        if (mode == Mode::Read)
            engine = std::make_shared<engine::SkeletonReader>(*this, name, mode,
                                                              mpiComm);
        else
            engine = std::make_shared<engine::SkeletonWriter>(*this, name, mode,
                                                              mpiComm);
    }
    else if (engineTypeLC == "inline")
    {
        if (mode == Mode::Read)
            engine = std::make_shared<engine::InlineReader>(*this, name, mode,
                                                            mpiComm);
        else
            engine = std::make_shared<engine::InlineWriter>(*this, name, mode,
                                                            mpiComm);
    }
    else
    {
        if (m_DebugMode)
        {
            throw std::invalid_argument("ERROR: engine " + m_EngineType +
                                        " not supported, IO SetEngine must add "
                                        "a supported engine, in call to "
                                        "Open\n");
        }
    }

    auto itEngine = m_Engines.emplace(name, std::move(engine));

    if (m_DebugMode)
    {
        if (!itEngine.second)
        {
            throw std::invalid_argument(
                "ERROR: engine of type " + m_EngineType + " and name " + name +
                " could not be created, in call to Open\n");
        }
    }
    // return a reference
    return *itEngine.first->second.get();
}

Engine &IO::Open(const std::string &name, const Mode mode)
{
    return Open(name, mode, m_MPIComm);
}

Engine &IO::GetEngine(const std::string &name)
{
    auto itEngine = m_Engines.find(name);
    if (m_DebugMode)
    {
        if (itEngine == m_Engines.end())
        {
            throw std::invalid_argument(
                "ERROR: engine name " + name +
                " could not be found, in call to GetEngine\n");
        }
    }
    // return a reference
    return *itEngine->second.get();
}

void IO::FlushAll()
{
    for (auto &enginePair : m_Engines)
    {
        auto &engine = enginePair.second;
        if (engine->OpenMode() != Mode::Read)
        {
            enginePair.second->Flush();
        }
    }
}

struct IO::ResetStepSelection
{
    template <class T>
    void operator()(Variable<T> &variable, bool zeroStart,
                    const std::string &hint)
    {
        variable.CheckRandomAccessConflict(hint);
        variable.ResetStepsSelection(zeroStart);
        variable.m_RandomAccess = false;
    }
};

void IO::ResetVariablesStepSelection(const bool zeroStart,
                                     const std::string hint)
{
    for (auto &variable : GetVariablesDataMap().range())
    {
        variable.Visit(ResetStepSelection(), zeroStart, hint);
    }
}

void IO::LockDefinitions() noexcept { m_DefinitionsLocked = true; };

// PRIVATE
std::string IO::AttributeGlobalName(const std::string &name,
                                    const std::string &variableName,
                                    const std::string separator) const
{
    if (m_DebugMode)
    {
        if (!variableName.empty() &&
            InquireVariableType(variableName) == DataType::Unknown)
        {
            throw std::invalid_argument(
                "ERROR: variable " + variableName +
                " doesn't exist, can't associate attribute " + name +
                ", in call to DefineAttribute");
        }
    }

    const std::string globalName =
        helper::GlobalName(name, variableName, separator);

    return globalName;
}

void IO::CheckTransportType(const std::string type) const
{
    if (type.empty() || type.find("=") != type.npos)
    {
        throw std::invalid_argument(
            "ERROR: wrong first argument " + type +
            ", must "
            "be a single word for a supported transport type, in "
            "call to IO AddTransport \n");
    }
}

// Explicitly instantiate the necessary public template implementations
#define define_template_instantiation(T)                                       \
    template Variable<T> &IO::DefineVariable<T>(const std::string &,           \
                                                const Dims &, const Dims &,    \
                                                const Dims &, const bool);     \
    template Variable<T> *IO::InquireVariable<T>(const std::string &) noexcept;

ADIOS2_FOREACH_STDTYPE_1ARG(define_template_instantiation)
#undef define_template_instatiation

#define declare_template_instantiation(T)                                      \
    template Attribute<T> &IO::DefineAttribute<T>(                             \
        const std::string &, const T *, const size_t, const std::string &,     \
        const std::string);                                                    \
    template Attribute<T> &IO::DefineAttribute<T>(                             \
        const std::string &, const T &, const std::string &,                   \
        const std::string);                                                    \
    template Attribute<T> *IO::InquireAttribute<T>(                            \
        const std::string &, const std::string &, const std::string) noexcept;

ADIOS2_FOREACH_ATTRIBUTE_STDTYPE_1ARG(declare_template_instantiation)
#undef declare_template_instantiation

} // end namespace core
} // end namespace adios2
