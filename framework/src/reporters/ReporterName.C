//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterName.h"
#include "MooseError.h"

const std::string ReporterName::REPORTER_RESTARTABLE_DATA_PREFIX = "ReporterData";

ReporterName::ReporterName(const std::string & object_name, const std::string & value_name)
  : _object_name(object_name), _value_name(value_name)
{
}

ReporterName::ReporterName(const std::string & combined_name)
{
  std::size_t idx = combined_name.rfind("/");
  if (idx != std::string::npos)
  {
    _object_name = combined_name.substr(0, idx);
    _value_name = combined_name.substr(idx + 1);
  }
  else
    mooseError("Invalid combined Reporter name: ", combined_name);
}

ReporterName::ReporterName(const char * combined_name) : ReporterName(std::string(combined_name)) {}

const std::string &
ReporterName::getObjectName() const
{
  return _object_name;
}

const std::string &
ReporterName::getValueName() const
{
  return _value_name;
}

const std::string
ReporterName::getCombinedName() const
{
  return _object_name + "/" + _value_name;
}

std::string
ReporterName::getRestartableName() const
{
  return REPORTER_RESTARTABLE_DATA_PREFIX + "/" + getCombinedName();
}

ReporterName::operator std::string() const { return getCombinedName(); }

bool
ReporterName::operator==(const ReporterName & rhs) const
{
  // Note here that we do not check if _special_type is the same. This is because
  // we want to store reporter names as a single name regardless of the type
  return _object_name == rhs._object_name && _value_name == rhs._value_name;
}

bool
ReporterName::operator==(const std::string & combined_name) const
{
  return getCombinedName() == combined_name;
}

bool
ReporterName::operator<(const ReporterName & rhs) const
{
  // Note here that we do not sort by _special_type. This is because
  // we want to store reporter names as a single name regardless of the type
  return getCombinedName() < rhs.getCombinedName();
}

std::string
ReporterName::specialTypeToName() const
{
  if (isPostprocessor())
    return "Postprocessor";
  if (isVectorPostprocessor())
    return "VectorPostprocessor";
  return "Reporter";
}

std::ostream &
operator<<(std::ostream & os, const ReporterName & state)
{
  os << state.getCombinedName();
  return os;
}

PostprocessorReporterName::PostprocessorReporterName(const PostprocessorName & name)
  : ReporterName(name, "value")
{
  setIsPostprocessor();
}

VectorPostprocessorReporterName::VectorPostprocessorReporterName(
    const VectorPostprocessorName & name, const std::string & vector_name)
  : ReporterName(name, vector_name)
{
  setIsVectorPostprocessor();
}
