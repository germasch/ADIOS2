/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * IO.h factory class of Parameters, Variables, Transports to Engines
 *
 *  Created on: Dec 16, 2016
 *      Author: William F Godoy godoywf@ornl.gov
 */

#ifndef ADIOS2_CORE_IO_H_
#define ADIOS2_CORE_IO_H_

#include "variant/include/mapbox/variant.hpp"

/// \cond EXCLUDE_FROM_DOXYGEN
#include <map>
#include <memory> //std:shared_ptr
#include <string>
#include <unordered_map>
#include <utility> //std::pair
#include <vector>
#include <iostream>
/// \endcond

#include "adios2/ADIOSConfig.h"
#include "adios2/ADIOSMPICommOnly.h"
#include "adios2/ADIOSMacros.h"
#include "adios2/ADIOSTypes.h"
#include "adios2/core/ADIOS.h"
#include "adios2/core/Attribute.h"
#include "adios2/core/Variable.h"
#include "adios2/core/VariableCompound.h"
#include <adios2/helper/TypeList.h>

namespace tl = adios2::helper::tl;

namespace detail
{

} // namespace detail

// index sequence only
template <std::size_t ...>
struct indexSequence
 { };

template <std::size_t N, std::size_t ... Next>
struct indexSequenceHelper : public indexSequenceHelper<N-1U, N-1U, Next...>
 { };

template <std::size_t ... Next>
struct indexSequenceHelper<0U, Next ... >
 { using type = indexSequence<Next ... >; };

template <std::size_t N>
using makeIndexSequence = typename indexSequenceHelper<N>::type;

// tuple_fold

namespace tuple_impl_detail
{
template <bool ReverseIteration, typename... Elements, typename N_aryOp,
          typename... Args, size_t... Is>
void tuple_fold_impl(
    std::tuple<Elements...> &tupull, N_aryOp &&op,
    indexSequence<Is...> /*meta*/,
    Args
        &... args) noexcept(noexcept(static_cast<void>(std::initializer_list<char>{
    (static_cast<void>(op(
         std::get<(ReverseIteration ? sizeof...(Elements)-1 - Is : Is)>(tupull),
         args...)),
     '0')...})))
{
    constexpr size_t tuple_size = sizeof...(Elements);
    static_cast<void>(std::initializer_list<char>{
        (static_cast<void>(
             op(std::get<(ReverseIteration ? tuple_size - 1 - Is : Is)>(tupull),
                args...)),
         '0')...});
}
}

template <bool ReverseIteration = false, typename... Elements, typename N_aryOp,
          typename... Args>
void tuple_fold(
    std::tuple<Elements...> &tuple, N_aryOp &&op,
    Args &&... args) noexcept(noexcept(tuple_impl_detail::
                                           tuple_fold_impl<ReverseIteration>(
                                               tuple, std::forward<N_aryOp>(op),
                                               makeIndexSequence<
                                                   sizeof...(Elements)>{},
                                               args...)))
{
    tuple_impl_detail::tuple_fold_impl<ReverseIteration>(
        tuple, std::forward<N_aryOp>(op),
        makeIndexSequence<sizeof...(Elements)>{}, args...);
}

struct monostate
{
};

template <class T>
struct add_reference_wrapper_impl
{
    using type = std::reference_wrapper<T>;
};

template <class T>
using add_reference_wrapper = typename add_reference_wrapper_impl<T>::type;

namespace adios2
{

namespace core
{

// ======================================================================
// EntityTuple

template <template <class> class Entity>
struct EntityTuple;

template <>
struct EntityTuple<Variable>
{
    using type =
        std::tuple<std::string, int8_t, int16_t, int32_t, int64_t, uint8_t,
                   uint16_t, uint32_t, uint64_t, float, double, long double,
                   std::complex<float>, std::complex<double>, Compound>;
};

template <>
struct EntityTuple<Attribute>
{
    using type =
        std::tuple<std::string, int8_t, int16_t, int32_t, int64_t, uint8_t,
                   uint16_t, uint32_t, uint64_t, float, double, long double>;
};

/** used for Variables and Attributes, name, type, type-index */

template <template <class> class Entity, class T>
class EntityMap
{
public:
    using Index = unsigned int;
    using Value = Entity<T>;
    using Map = std::map<Index, Value>;
    using iterator = typename Map::iterator;

    void erase(Index key);
    void clear() noexcept;

    Value &at(Index key);
    const Value &at(Index key) const;

    template <class... Args>
    iterator emplace(Args &&... args);

    static DataType GetType()
    {
        return helper::GetType<T>();
    } // FIXME, should do something constexpr

private:
    Map m_Map;
    Index m_Index = 0;
};

template <template <class> class Entity>
struct EntityBase;

template <>
struct EntityBase<Variable>
{
    using type = VariableBase;
};

template <>
struct EntityBase<Attribute>
{
    using type = AttributeBase;
};

// ======================================================================
// DataMap

// Entity is either Variable or Attribute
template <template <class> class Entity>
class DataMap
{
    using Index = unsigned int;
    using Value = std::pair<DataType, Index>;
    using NameMap = std::unordered_map<std::string, Value>;
    using iterator = NameMap::iterator;

public:
    using const_iterator = NameMap::const_iterator;

    using Entities = tl::Transform<Entity, typename EntityTuple<Entity>::type>;
    // std::tuple<Entity<int8_t>, Entity<int16_t>, ...>
    using EntityRefVariant =
        tl::Apply<mapbox::util::variant,
                  tl::PushFront<monostate, tl::Transform<add_reference_wrapper,
                                                         Entities>>>;
    // e.g., <monostate, Variable<int8_t>&, Variable<int16_t>&, ...>

    template <typename T>
    using EntityMapForT = EntityMap<Entity, T>;
    using EntityMaps =
        tl::Transform<EntityMapForT, typename EntityTuple<Entity>::type>;
    // e.g., std::tuple<VariableMap<int8_t>, VariableMap<int16_t>, ...>
    using EntityMapRefVariant =
        tl::Apply<mapbox::util::variant,
                  tl::PushFront<monostate, tl::Transform<add_reference_wrapper,
                                                         EntityMaps>>>;
    // e.g., variant<monostate, VariableMap<int8_t>&, VariableMap<int16_t>&,
    // ...>
    using EntityBase = typename EntityBase<Entity>::type;

    class Range
    {
        using value_type = EntityBase;

    public:
        Range(const DataMap<Entity> &map) : m_Map(map) {}

        struct const_iterator
            : std::iterator<std::forward_iterator_tag, value_type>
        {
            const_iterator(NameMap::const_iterator it,
                           const DataMap<Entity> &map)
            : m_It{it}, m_Map{map}
            {
            }

            bool operator==(const_iterator other) const
            {
                return m_It == other.m_It;
            }
            bool operator!=(const_iterator other) const
            {
                return !(*this == other);
            }

            const_iterator &operator++()
            {
                m_It++;
                return *this;
            }
            const_iterator operator++(int)
            {
                auto retval = *this;
                ++(*this);
                return retval;
            }
            const value_type &operator*()
            {
                const std::string name = m_It->first;
                DataType type = m_It->second.first;
                Index index = m_It->second.second;

                EntityBase *variable = nullptr;
                if (false)
                {
                }
#define declare_template_instantiation(T)                                      \
    else if (type == helper::GetType<T>())                                     \
    {                                                                          \
        auto &map = const_cast<DataMap<Entity> &>(m_Map).GetEntityMap<T>();    \
        variable = &map.at(index);                                             \
    }
                ADIOS2_FOREACH_ATTRIBUTE_STDTYPE_1ARG(
                    declare_template_instantiation)
#undef declare_template_instantiation
                // FIXME!!!!!!!!!!!!!!!!!!! skips type for variable

                assert(variable);
                return *variable;
            }

            const value_type *operator->() { return &operator*(); }

        private:
            NameMap::const_iterator m_It;
            const DataMap<Entity> &m_Map;
        };

        const_iterator begin() const noexcept { return {m_Map.begin(), m_Map}; }
        const_iterator end() const noexcept { return {m_Map.end(), m_Map}; }

        size_t size() const noexcept { return m_Map.size(); };

        const_iterator find(const std::string &name)
        {
            auto itMap = m_Map.find(name);
            return {itMap, m_Map};
        }

        const DataMap<Entity> &m_Map;
    };

    Range range() const { return {*this}; }

    template <class Visitor, class EntityBase, class... Args>
    static void visit(Visitor &&visitor, EntityBase &entityBase,
                      Args &&... args)
    {
        const DataType type = entityBase.m_Type;

        if (false)
        {
        }
#define declare_template_instantiation(T)                                      \
    else if (type == helper::GetType<T>())                                     \
    {                                                                          \
        auto &entity = dynamic_cast<const Entity<T> &>(entityBase);            \
        visitor(entity, std::forward<Args>(args)...);                          \
    }
        ADIOS2_FOREACH_STDTYPE_1ARG(declare_template_instantiation)
#undef declare_template_instantiation
    }

    template <class Visitor, class... Args>
    void foreach (Visitor &&visitor, Args && ... args)
    {
        for (auto var : range())
        {
            visit(std::forward<Visitor>(visitor), var,
                  std::forward<Args>(args)...);
        }
    }

    iterator begin() noexcept { return m_NameMap.begin(); }
    const_iterator begin() const noexcept { return m_NameMap.begin(); }
    iterator end() noexcept { return m_NameMap.end(); }
    const_iterator end() const noexcept { return m_NameMap.end(); }
    const_iterator find(const std::string &name) const { return m_NameMap.find(name); }

    const Value &at(const std::string &name) const { return m_NameMap.at(name); }

    template <class T, class... Args>
    Entity<T> &emplace(const std::string &name, Args &&... args)
    {
        auto itNameMap = m_NameMap.find(name);
        if (itNameMap != m_NameMap.end())
        {
            throw std::invalid_argument(
                "ERROR: Entity " + name +
                " exists, in call to DefineVariable/Attribute\n");
        }
        auto &entityMap = GetEntityMap<T>();
        auto it = entityMap.emplace(name, std::forward<Args>(args)...);
        Index index = it->first;
        Entity<T> &entity = it->second;
        m_NameMap.emplace(entity.m_Name,
                          std::make_pair(helper::GetType<T>(), index));
        return entity;
    }

    struct DoClear
    {
        template <typename T>
        void operator()(T &map) noexcept
        {
            map.clear();
        }
    };

    void RemoveAll() noexcept
    {
        m_NameMap.clear();
        tuple_fold(m_EntityMaps, DoClear{});
    }
    size_t size() const noexcept { return m_NameMap.size(); };

    template <class... Args>
    std::pair<iterator, bool> emplace(Args &&... args)
    {
        return m_NameMap.emplace(std::forward<Args>(args)...);
    }

    template <class T>
    EntityMapForT<T> &GetEntityMap() noexcept
    {
        return helper::GetByType<EntityMapForT<T>>(m_EntityMaps);
    }

    struct SetIfType
    {
        SetIfType(EntityMapRefVariant &entityMap, DataType type)
        : m_EntityMap(entityMap), m_Type(type)
        {
        }

        template <typename EntityMap>
        void operator()(EntityMap &map)
        {
            if (m_Type == EntityMap::GetType())
            {
                m_EntityMap = std::ref(map);
            }
        }

        EntityMapRefVariant &m_EntityMap;
        DataType m_Type;
    };

    EntityMapRefVariant GetEntityMap(DataType type) const /* FIXME, const */
    {
        EntityMapRefVariant entityMap;
        tuple_fold(const_cast<EntityMaps &>(m_EntityMaps),
                   SetIfType{entityMap, type});
        // FIXME, could assert that found
        return entityMap;
    }

    template <class F>
    struct SkipMonoState
    {
        SkipMonoState(const F &f) : m_F(f) {}

        template <class T>
        void operator()(T &t)
        {
            m_F(t);
        }

        void operator()(monostate s) {}

        F m_F; // FIXME, function object gets copied
    };

    template <class F>
    void visit(F &&f, DataType type)
    {
        auto entityMapV = GetEntityMap(type);
        mapbox::util::apply_visitor(SkipMonoState<F>(std::forward<F>(f)),
                                    entityMapV);
    }

    struct DoErase
    {
        DoErase(unsigned int index) : m_Index(index) {}

        template <typename Map>
        void operator()(Map &map)
        {
            map.erase(m_Index);
        }

        unsigned int m_Index;
    };

    bool Remove(const std::string &name)
    {
        auto it = m_NameMap.find(name);
        // doesn't exist?
        if (it == m_NameMap.end())
        {
            return false;
        }

        // first remove it from EntityMap
        const DataType type(it->second.first);
        const Index index(it->second.second);

        visit(DoErase{index}, type);

        // then from NameMap
        m_NameMap.erase(name);

        return true;
    }

    template <typename T>
    Entity<T> *Find(const std::string &name)
    {
        auto it = m_NameMap.find(name);
        // doesn't exist?
        if (it == m_NameMap.end())
        {
            return nullptr;
        }
        const DataType type(it->second.first);
        const Index index(it->second.second);
        if (type != helper::GetType<T>())
        {
            return nullptr;
        }

        auto& entityMap = GetEntityMap<T>();
        return &entityMap.at(index);
    }

    struct DoAt
    {
        DoAt(unsigned int index, EntityRefVariant &entityRefV)
        : m_Index(index), m_EntityRefV(entityRefV)
        {
        }

        template <typename Map>
        void operator()(Map &map)
        {
            m_EntityRefV = std::ref(map.at(m_Index));
        }

        unsigned int m_Index;
        EntityRefVariant &m_EntityRefV;
    };

    EntityRefVariant
    FindV(const std::string &name) const /* FIXME return const ref */
    {
        auto it = m_NameMap.find(name);
        // doesn't exist?
        if (it == m_NameMap.end())
        {
            return {};
        }
        const DataType type(it->second.first);
        const Index index(it->second.second);

        auto entityMapV = GetEntityMap(type);
        EntityRefVariant entityRef;
        mapbox::util::apply_visitor(SkipMonoState<DoAt>(DoAt{index, entityRef}),
                                    entityMapV);
        return entityRef;
    }

private:
    NameMap m_NameMap;

public:
    EntityMaps m_EntityMaps;
};

template <class Visitor, class... Args>
void visit(Visitor &&visitor, AttributeBase *var, Args &&... args)
{
    const DataType type = var->m_Type;

    if (false)
    {
    }
#define declare_template_instantiation(T)                                      \
    else if (type == helper::GetType<T>())                                     \
    {                                                                          \
        Attribute<T> &attribute = dynamic_cast<Attribute<T> &>(*var);          \
        visitor(attribute, std::forward<Args>(args)...);                       \
    }
    ADIOS2_FOREACH_ATTRIBUTE_STDTYPE_1ARG(declare_template_instantiation)
#undef declare_template_instantiation
}

template <class T>
using VariableMap = DataMap<Variable>::EntityMapForT<T>;
template <class T>
using AttributeMap = DataMap<Attribute>::EntityMapForT<T>;

using VariableMaps = DataMap<Variable>::EntityMaps;
using AttributeMaps = DataMap<Attribute>::EntityMaps;

// forward declaration needed as IO is passed to Engine derived
// classes
class Engine;

/** Factory class IO for settings, variables, and transports to an engine */
class IO
{

public:
    /** reference to object that created current IO */
    ADIOS &m_ADIOS;

    /** unique identifier */
    const std::string m_Name;

    /** from ADIOS class passed to Engine created with Open
     *  if no new communicator is passed */
    MPI_Comm m_MPIComm;

    /** true: extra exceptions checks */
    const bool m_DebugMode = false;

    /** from ADIOS class passed to Engine created with Open */
    const std::string m_HostLanguage = "C++";

    /** From SetParameter, parameters for a particular engine from m_Type */
    Params m_Parameters;

    /** From AddTransport, parameters in map for each transport in vector */
    std::vector<Params> m_TransportsParameters;

    /** Carries information about operations added with AddOperation */
    struct Operation
    {
        Operator *Op;
        Params Parameters;
        Params Info;
    };

    /** From AddOperation, contains operators added to this IO */
    std::vector<Operation> m_Operations;

    /** BP3 engine default if unknown */
    std::string m_EngineType = "BP3";

    /** at read for file engines: true: in streaming (step-by-step) mode, or
     * false: random-access mode (files) */
    bool m_ReadStreaming = false;

    /** used if m_Streaming is true by file reader engines */
    size_t m_EngineStep = 0;

    /** placeholder when reading XML file variable operations, executed until
     * DefineVariable in code */
    std::map<std::string, std::vector<Operation>> m_VarOpsPlaceholder;

    /** true: No more definitions or changes to existing variables are allowed
     */
    bool m_DefinitionsLocked = false;

    /**
     * @brief Constructor called from ADIOS factory class DeclareIO function.
     * Not to be used direclty in applications.
     * @param adios reference to ADIOS object that owns current IO
     * @param name unique identifier for this IO object
     * @param mpiComm MPI communicator from ADIOS factory class
     * @param inConfigFile IO defined in config file (XML)
     * @param hostLanguage current language using the adios2 library
     * @param debugMode true: extra exception checks (recommended)
     */
    IO(ADIOS &adios, const std::string name, MPI_Comm mpiComm,
       const bool inConfigFile, const std::string hostLanguage,
       const bool debugMode);

    ~IO() = default;

    /**
     * @brief Sets the engine type for this IO class object
     * @param engine predefined engine type, default is bpfile
     */
    void SetEngine(const std::string engine) noexcept;

    /**
     * @brief Set the IO mode (collective or independent), not yet implemented
     * @param IO mode */
    void SetIOMode(const IOMode mode);

    /**
     * @brief Version that passes a map to fill out parameters
     * initializer list = { "param1", "value1" },  {"param2", "value2"},
     * @param params adios::Params std::map<std::string, std::string>
     */
    void SetParameters(const Params &parameters = Params()) noexcept;

    /**
     * @brief Sets a single parameter overwriting value if key exists;
     * @param key parameter key
     * @param value parameter value
     */
    void SetParameter(const std::string key, const std::string value) noexcept;

    /** @brief Retrieve current parameters map */
    Params &GetParameters() noexcept;

    /**
     * @brief Adds a transport and its parameters for the IO Engine
     * @param type must be a supported transport type
     * @param params acceptable parameters for a particular transport
     * @return transportIndex handler
     */
    size_t AddTransport(const std::string type,
                        const Params &parameters = Params());

    /**
     * @brief Sets a single parameter to an existing transport identified with a
     * transportIndex handler from AddTransport.
     * This function overwrites existing parameter with the same key.
     * @param transportIndex index handler from AddTransport
     * @param key parameter key
     * @param value parameter value
     */
    void SetTransportParameter(const size_t transportIndex,
                               const std::string key, const std::string value);

    /**
     * @brief Define a Variable of primitive data type for current IO.
     * Default (name only) is a local single value,
     * in order to be compatible with ADIOS1.
     * @param name variable name, must be unique within Method
     * @param shape overall dimensions e.g. {Nx*size, Ny*size, Nz*size}
     * @param start point (offset) for MPI rank e.g. {Nx*rank, Ny*rank, Nz*rank}
     * @param count length for MPI rank e.g. {Nx, Ny, Nz}
     * @param constantShape true if dimensions, offsets and local sizes don't
     * change over time
     * @return reference to Variable object
     * @exception std::invalid_argument if Variable with unique name is already
     * defined, in debug mode only
     */
    template <class T>
    Variable<T> &
    DefineVariable(const std::string &name, const Dims &shape = Dims(),
                   const Dims &start = Dims(), const Dims &count = Dims(),
                   const bool constantDims = false);

    /**
     * @brief Define array attribute
     * @param name must be unique for the IO object
     * @param array pointer to user data
     * @param elements number of data elements
     * @param variableName optionally associates the attribute to a Variable
     * @return reference to internal Attribute
     * @exception std::invalid_argument if Attribute with unique name is already
     * defined, in debug mode only
     */
    template <class T>
    Attribute<T> &DefineAttribute(const std::string &name, const T *array,
                                  const size_t elements,
                                  const std::string &variableName = "",
                                  const std::string separator = "/");

    /**
     * @brief Define single value attribute
     * @param name must be unique for the IO object
     * @param value single data value
     * @return reference to internal Attribute
     * @exception std::invalid_argument if Attribute with unique name is already
     * defined, in debug mode only
     */
    template <class T>
    Attribute<T> &DefineAttribute(const std::string &name, const T &value,
                                  const std::string &variableName = "",
                                  const std::string separator = "/");

    /**
     * @brief Removes an existing Variable in current IO object.
     * Dangerous function since references and
     * pointers can be dangling after this call.
     * @param name unique identifier input
     * @return true: found and removed variable, false: not found, nothing to
     * remove
     */
    bool RemoveVariable(const std::string &name) noexcept;

    /**
     * @brief Removes all existing variables in current IO object.
     * Dangerous function since references and
     * pointers can be dangling after this call.
     */
    void RemoveAllVariables() noexcept;

    /**
     * @brief Removes an existing Attribute in current IO object.
     * Dangerous function since references and
     * pointers can be dangling after this call.
     * @param name unique identifier input
     * @return true: found and removed attribute, false: not found, nothing to
     * remove
     */
    bool RemoveAttribute(const std::string &name) noexcept;

    /**
     * @brief Removes all existing attributes in current IO object.
     * Dangerous function since references and
     * pointers can be dangling after this call.
     */
    void RemoveAllAttributes() noexcept;

    /**
     * @brief Retrieve map with variables info. Use when reading.
     * @return map with current variables and info
     * keys: Type, Min, Max, Value, AvailableStepsStart,
     * AvailableStepsCount, Shape, Start, Count, SingleValue
     */
    std::map<std::string, Params> GetAvailableVariables() noexcept;

    /**
     * @brief Gets an existing variable of primitive type by name
     * @param name of variable to be retrieved
     * @return pointer to an existing variable in current IO, nullptr if not
     * found
     */
    template <class T>
    Variable<T> *InquireVariable(const std::string &name) noexcept;

    /**
     * @brief Returns the type of an existing variable as an string
     * @param name input variable name
     * @return type primitive type
     */
    DataType InquireVariableType(const std::string &name) const noexcept;

    /**
     * Retrieves hash holding internal variable identifiers
     * @return
     * <pre>
     * key: unique variable name,
     * value: pair.first = DataType type
     *        pair.second = order in the type bucket
     * </pre>
     */
    const DataMap<Variable> &GetVariablesDataMap() const noexcept;

    /**
     * Retrieves hash holding internal Attributes identifiers
     * @return
     * <pre>
     * key: unique attribute name,
     * value: pair.first = DataType type
     *        pair.second = order in the type bucket
     * </pre>
     */
    const DataMap<Attribute> &GetAttributesDataMap() const noexcept;

    /**
     * Gets an existing attribute of primitive type by name
     * @param name of attribute to be retrieved
     * @return pointer to an existing attribute in current IO, nullptr if not
     * found
     */
    template <class T>
    Attribute<T> *InquireAttribute(const std::string &name,
                                   const std::string &variableName = "",
                                   const std::string separator = "/") noexcept;

    /**
     * @brief Returns the type of an existing attribute as an string
     * @param name input attribute name
     * @return type if found returns type as string, otherwise an empty string
     */
    DataType InquireAttributeType(const std::string &name,
                                  const std::string &variableName = "",
                                  const std::string separator = "/") const
        noexcept;

    /**
     * @brief Retrieve map with attributes info. Use when reading.
     * @return map with current attributes and info
     * keys: Type, Elements, Value
     */
    std::map<std::string, Params>
    GetAvailableAttributes(const std::string &variableName = std::string(),
                           const std::string separator = "/") noexcept;

    /**
     * @brief Check existence in config file passed to ADIOS class constructor
     * @return true: defined in config file, false: not found in config file
     */
    bool InConfigFile() const noexcept;

    /**
     * Sets declared to true if IO exists in code created with ADIOS DeclareIO
     */
    void SetDeclared() noexcept;

    /**
     * Check if declared in code
     * @return true: created with ADIOS DeclareIO, false: dummy from config file
     */
    bool IsDeclared() const noexcept;

    /**
     * Adds an operator defined by the ADIOS class. Could be a variable set
     * transform, callback function, etc.
     * @param op operator created by the ADIOS class
     * @param parameters specific parameters for current IO
     * @return operation handler
     */
    size_t AddOperation(Operator &op,
                        const Params &parameters = Params()) noexcept;

    /**
     * @brief Creates a polymorphic object that derives the Engine class,
     * based on the SetEngine function or config file input
     * @param name unique engine identifier within IO object
     * (e.g. file name in case of Files)
     * @param mode write, read, append from ADIOSTypes.h Mode
     * @param mpiComm assigns a new communicator to the Engine
     * @return a reference to a derived object of the Engine class
     * @exception std::invalid_argument if Engine with unique name is already
     * created with another Open, in debug mode only
     */
    Engine &Open(const std::string &name, const Mode mode, MPI_Comm mpiComm);

    /**
     * Overloaded version that reuses the MPI_Comm object passed
     * from the ADIOS class to the IO class
     * @param name unique engine identifier within IO object
     * (file name in case of File transports)
     * @param mode write, read, append from ADIOSTypes.h OpenMode
     * @return a reference to a derived object of the Engine class
     * @exception std::invalid_argument if Engine with unique name is already
     * created with another Open, in debug mode only
     */
    Engine &Open(const std::string &name, const Mode mode);

    /**
     * Retrieve an engine by name
     */
    Engine &GetEngine(const std::string &name);

    /**
     * Flushes all engines created with the current IO object using Open.
     * If no engine is created it does nothing.
     * @exception std::runtime_error if any engine Flush fails
     */
    void FlushAll();

    // READ FUNCTIONS, not yet implemented:
    /**
     * not yet implented
     * @param pattern
     */
    void SetReadMultiplexPattern(const ReadMultiplexPattern pattern);

    /**
     * not yet implemented
     * @param mode
     */
    void SetStreamOpenMode(const StreamOpenMode mode);

    /**
     * Resets all variables m_StepsStart and m_StepsCount
     * @param alwaysZero true: always m_StepsStart = 0, false: capture
     */
    void ResetVariablesStepSelection(const bool zeroStart = false,
                                     const std::string hint = "");

    /**
     * @brief Promise that no more definitions or changes to defined variables
     * will occur. Useful information if called before the first EndStep() of an
     * output Engine, as it will know that the definitions are complete and
     * constant for the entire lifetime of the output and may optimize metadata
     * handling.
     */
    void LockDefinitions() noexcept;

private:
    struct AddAvailableVariable;
    struct AddAvailableAttribute;

    /** true: exist in config file (XML) */
    const bool m_InConfigFile = false;

    bool m_IsDeclared = false;

    /** Independent (default) or Collective */
    adios2::IOMode m_IOMode = adios2::IOMode::Independent;

    // Variables
    /**
     * Map holding variable identifiers
     * <pre>
     * key: unique variable name,
     * value: pair.first = type as string GetType<T> from adiosTemplates.h
     *        pair.second = index in fixed size map (e.g. m_Int8, m_Double)
     * </pre>
     */
    DataMap<Variable> m_Variables;

    /** Variable containers based on fixed-size type */

    /** Gets the internal reference to a variable map for type T
     *  This function is specialized in IO.tcc */
    template <class T>
    VariableMap<T> &GetVariableMap() noexcept
    {
        return m_Variables.GetEntityMap<T>();
    }

    /**
     * Map holding attribute identifiers
     * <pre>
     * key: unique attribute name,
     * value: pair.first = type as string GetType<T> from
     *                     helper/adiosTemplates.h
     *        pair.second = index in fixed size map (e.g. m_Int8, m_Double)
     * </pre>
     */
    DataMap<Attribute> m_Attributes;

    template <class T>
    AttributeMap<T> &GetAttributeMap() noexcept
    {
        return m_Attributes.GetEntityMap<T>();
    }

    std::map<std::string, std::shared_ptr<Engine>> m_Engines;

    /** Gets global name by combining variableName and (attribute) name
     */
    std::string AttributeGlobalName(const std::string &name,
                                    const std::string &variableName,
                                    const std::string separator) const;

    void CheckTransportType(const std::string type) const;

    template <class T>
    bool IsAvailableStep(const size_t step,
                         const unsigned int variableIndex) noexcept;
};

// Explicit declaration of the public template methods
#define declare_template_instantiation(T)                                      \
    extern template Variable<T> &IO::DefineVariable<T>(                        \
        const std::string &, const Dims &, const Dims &, const Dims &,         \
        const bool);                                                           \
    extern template Variable<T> *IO::InquireVariable<T>(                       \
        const std::string &name) noexcept;

ADIOS2_FOREACH_STDTYPE_1ARG(declare_template_instantiation)
#undef declare_template_instantiation

#define declare_template_instantiation(T)                                      \
    extern template Attribute<T> &IO::DefineAttribute<T>(                      \
        const std::string &, const T *, const size_t, const std::string &,     \
        const std::string);                                                    \
    extern template Attribute<T> &IO::DefineAttribute<T>(                      \
        const std::string &, const T &, const std::string &,                   \
        const std::string);                                                    \
    extern template Attribute<T> *IO::InquireAttribute<T>(                     \
        const std::string &, const std::string &, const std::string) noexcept;

ADIOS2_FOREACH_ATTRIBUTE_STDTYPE_1ARG(declare_template_instantiation)
#undef declare_template_instantiation

} // end namespace core
} // end namespace adios2

#include "IO.inl"

#endif /* ADIOS2_CORE_IO_H_ */
