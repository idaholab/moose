/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MooseEnum.h"
#include "MooseUtils.h"
#include "MooseError.h"

#include <sstream>
#include <algorithm>
#include <iterator>
#include <limits>
#include <string>
#include <iostream>

MooseEnum::MooseEnum(std::string names, std::string default_name, bool allow_out_of_range)
  : MooseEnumBase(names, allow_out_of_range)
{
  *this = default_name;
}

MooseEnum::MooseEnum(const MooseEnum & other_enum)
  : MooseEnumBase(other_enum),
    _current_id(other_enum._current_id),
    _current_name(other_enum._current_name),
    _current_name_preserved(other_enum._current_name_preserved)
{
}

MooseEnum
MooseEnum::withNamesFrom(const MooseEnumBase & other_enum)
{
  return MooseEnum(other_enum);
}

/**
 * Private constuctor for use by libmesh::Parameters
 */
MooseEnum::MooseEnum() : _current_id(INVALID_ID) {}

MooseEnum::MooseEnum(const MooseEnumBase & other_enum) : MooseEnumBase(other_enum) {}

MooseEnum &
MooseEnum::operator=(const std::string & name)
{
  if (name == "")
  {
    _current_id = INVALID_ID;
    _current_name = "";
    _current_name_preserved = "";
    return *this;
  }

  std::string upper(name);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  _current_name = upper;
  _current_name_preserved = name;

  checkDeprecatedBase(upper);

  if (std::find(_names.begin(), _names.end(), upper) == _names.end())
  {
    if (_out_of_range_index == 0) // Are out of range values allowed?
      mooseError(std::string("Invalid option \"") + upper +
                 "\" in MooseEnum.  Valid options (not case-sensitive) are \"" + _raw_names +
                 "\".");
    else
    {
      // Allow values assigned outside of the enumeration range
      _names.push_back(upper);

      _current_id = _out_of_range_index++;
      _name_to_id[upper] = _current_id;
    }
  }
  else
    _current_id = _name_to_id[upper];

  return *this;
}

bool
MooseEnum::operator==(const char * name) const
{
  std::string upper(name);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  mooseAssert(
      _out_of_range_index != 0 || std::find(_names.begin(), _names.end(), upper) != _names.end(),
      std::string("Invalid string comparison \"") + upper +
          "\" in MooseEnum.  Valid options (not case-sensitive) are \"" + _raw_names + "\".");

  return _current_name == upper;
}

bool
MooseEnum::operator!=(const char * name) const
{
  return !(*this == name);
}

bool
MooseEnum::operator==(int value) const
{
  return value == _current_id;
}

bool
MooseEnum::operator!=(int value) const
{
  return value != _current_id;
}

bool
MooseEnum::operator==(unsigned short value) const
{
  return value == _current_id;
}

bool
MooseEnum::operator!=(unsigned short value) const
{
  return value != _current_id;
}

bool
MooseEnum::operator==(const MooseEnum & value) const
{
  return value._current_name == _current_name;
}

bool
MooseEnum::operator!=(const MooseEnum & value) const
{
  return value._current_name != _current_name;
}

void
MooseEnum::checkDeprecated() const
{
  checkDeprecatedBase(_current_name);
}

void
MooseEnum::removeEnumerationName(std::string name)
{
  std::transform(name.begin(), name.end(), name.begin(), ::toupper);
  if (name == _current_name)
    mooseError("The current enumeration value of ", name, " cannot be removed.");
  MooseEnumBase::removeEnumerationName(name);
}
