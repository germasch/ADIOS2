
/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * ADIOSTypes.cpp: implementation of enum-related functions
 *
 *  Created on: Feb 22, 2019
 *      Author: Kai Germaschewski <kai.germaschewski@unh.edu>
 */

#include "ADIOSTypes.h"

namespace adios2
{

namespace
{

std::map<ShapeID, std::string> MapShapeID2String = {
    {ShapeID::Unknown, "ShapeID::Unknown"},
    {ShapeID::GlobalValue, "ShapeID::GlobalValue"},
    {ShapeID::GlobalArray, "ShapeID::GlobalArray"},
    {ShapeID::JoinedArray, "ShapeID::JoinedArray"},
    {ShapeID::LocalValue, "ShapeID::LocalValue"},
    {ShapeID::LocalArray, "ShapeID::LocalArray"},
};

std::map<IOMode, std::string> MapIOMode2String = {
    {IOMode::Independent, "IOMode::Independent"},
    {IOMode::Collective, "IOMode::Collective"},
};

std::map<Mode, std::string> MapMode2String = {
    {Mode::Undefined, "Mode::Undefined"}, {Mode::Write, "Mode::Write"},
    {Mode::Read, "Mode::Read"},           {Mode::Append, "Mode::Append"},
    {Mode::Sync, "Mode::Sync"},           {Mode::Deferred, "Mode::Deferred"},
};

std::map<DataType, const std::string> MapDataType2String = {
    {DataType::Unknown, ""},
    {DataType::Compound, "compound"},
    {DataType::String, "string"},
    {DataType::Int8, "signed char"},
    {DataType::Int16, "short"},
    {DataType::Int32, "int"},
    {DataType::Int64, "long long int"},
    {DataType::UInt8, "unsigned char"},
    {DataType::UInt16, "unsigned short"},
    {DataType::UInt32, "unsigned int"},
    {DataType::UInt64, "unsigned long long int"},
    {DataType::Float, "float"},
    {DataType::Double, "double"},
    {DataType::LDouble, "long double"},
    {DataType::CFloat, "float complex"},
    {DataType::CDouble, "double complex"},
};

} // end anonymous namespace

std::string ToString(ShapeID value) { return MapShapeID2String.at(value); }

std::string ToString(IOMode value) { return MapIOMode2String.at(value); }

std::string ToString(Mode value) { return MapMode2String.at(value); }

std::string ToString(DataType value) { return MapDataType2String.at(value); }

} // end namespace adios2
