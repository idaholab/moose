//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterName.h"

ReporterName::ReporterName(const std::string & object_name, const std::string & value_name)
  : _object_name(object_name),
    _value_name(value_name),
    _combined_name(object_name + "/" + value_name)
{
}

ReporterName::ReporterName(const std::string & combined_name) : _combined_name(combined_name)
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

const std::string &
ReporterName::getCombinedName() const
{
  return _combined_name;
}

ReporterName::operator std::string() const { return _combined_name; }

bool
ReporterName::operator==(const ReporterName & rhs) const
{
  return _combined_name == rhs._combined_name;
}

bool
ReporterName::operator==(const std::string & combined_name) const
{
  return _combined_name == combined_name;
}

bool
ReporterName::operator<(const ReporterName & rhs) const
{
  return _combined_name < rhs._combined_name;
}

ReporterName::ReporterName(const ReporterName & other)
  : ReporterName(other._object_name, other._value_name)
{
}

ReporterName &
ReporterName::operator=(const ReporterName & other)
{
  _object_name = other._object_name;
  _value_name = other._value_name;
  _combined_name = other._combined_name;
  return *this;
}

std::ostream &
operator<<(std::ostream & os, const ReporterName & state)
{
  os << state.getCombinedName();
  return os;
}
