//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MooseObjectParameterName.h"
#include "MooseError.h"

// STL includes
#include <iostream>

MooseObjectParameterName::MooseObjectParameterName(const MooseObjectName & obj_name,
                                                   const std::string & param)
  : MooseObjectName(obj_name), _parameter(param)
{
  _combined = _tag + _name + _parameter;
}

MooseObjectParameterName::MooseObjectParameterName(const std::string & tag,
                                                   const std::string & name,
                                                   const std::string & param,
                                                   const std::string & separator)
  : MooseObjectName(tag, name, separator), _parameter(param)
{
  _combined += _parameter;
}

MooseObjectParameterName::MooseObjectParameterName(const MooseObjectParameterName & rhs)
  : MooseObjectName(rhs._tag, rhs._name, rhs._separator), _parameter(rhs._parameter)
{
  _combined = _tag + _name + _parameter;
}

MooseObjectParameterName::MooseObjectParameterName(std::string name) : MooseObjectName()
{
  // The tag precedes the :: (this is used in _moose_base::name and control_tag::name conventions)
  std::size_t idx = name.find("::");
  if (idx != std::string::npos)
  {
    _tag = name.substr(0, idx);
    name.erase(0, idx + 2);
    _separator = "::";
  }

  // Locate the param name
  idx = name.rfind("/");
  if (idx != std::string::npos)
  {
    _parameter = name.substr(idx + 1);
    name.erase(idx);
  }
  else // if a slash isn't located then the entire name must be the parameter
  {
    _parameter = name;
    name.erase();
  }

  // If there is a second slash, there is a syntax based tag: tag/object_name/param
  idx = name.rfind("/");
  if (idx != std::string::npos)
  {
    _name = name.substr(idx + 1);
    name.erase(idx);
    _tag = name;
  }

  // If content still exists in "name", then this must be the object name
  if (_name.empty() && !name.empty())
    _name = name;

  // Set the combined name for sorting
  _combined = _tag + _name + _parameter;
}

bool
MooseObjectParameterName::operator==(const MooseObjectParameterName & rhs) const
{
  if (MooseObjectName::operator==(rhs) &&
      (_parameter == rhs._parameter || _parameter == "*" || rhs._parameter == "*"))
    return true;
  return false;
}

bool
MooseObjectParameterName::operator==(const MooseObjectName & rhs) const
{
  return MooseObjectName::operator==(rhs);
}

bool
MooseObjectParameterName::operator!=(const MooseObjectParameterName & rhs) const
{
  return !(*this == rhs);
}

bool
MooseObjectParameterName::operator!=(const MooseObjectName & rhs) const
{
  return MooseObjectName::operator!=(rhs);
}

bool
MooseObjectParameterName::operator<(const MooseObjectParameterName & rhs) const
{
  return (_combined < rhs._combined);
}

std::ostream &
operator<<(std::ostream & stream, const MooseObjectParameterName & obj)
{
  if (obj._tag.empty() && obj._name.empty())
    return stream << obj._parameter;
  else if (obj._tag.empty())
    return stream << obj._name << "/" << obj._parameter;
  else if (obj._name.empty())
    return stream << obj._tag << obj._separator << obj._parameter;
  else
    return stream << obj._tag << obj._separator << obj._name << "/" << obj._parameter;
}

void
MooseObjectParameterName::check()
{
  MooseObjectName::check();
  if (_parameter.empty())
    mooseError("The supplied parameter name cannot be empty, to allow for any parameter name to be "
               "supplied use the '*' character.");
}
