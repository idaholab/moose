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

const int MooseEnum::INVALID_ID = std::numeric_limits<int>::min();

MooseEnum::MooseEnum(std::string names, std::string default_name, bool allow_out_of_range) :
    _out_of_range_index(allow_out_of_range ? INVALID_ID + 1 : 0)
{
  fillNames(names);

  if (default_name == "")
    _current_id = INVALID_ID;
  else
    *this = default_name;
}

MooseEnum::MooseEnum(const MooseEnum & other_enum) :
    _names(other_enum._names),
    _raw_names(other_enum._raw_names),
    _raw_names_no_commas(other_enum._raw_names_no_commas),
    _name_to_id(other_enum._name_to_id),
    _current_id(other_enum._current_id),
    _current_name(other_enum._current_name),
    _current_name_preserved(other_enum._current_name_preserved),
    _out_of_range_index(other_enum._out_of_range_index)
{
}

/**
 * Private constuctor for use by libmesh::Parameters
 */
MooseEnum::MooseEnum() :
    _current_id(INVALID_ID)
{
}

MooseEnum &
MooseEnum::operator=(const std::string & name)
{
  std::string upper(name);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  _current_name = upper;
  _current_name_preserved = name;

  if (std::find(_names.begin(), _names.end(), upper) == _names.end())
  {
    if (_out_of_range_index == 0)     // Are out of range values allowed?
      mooseError(std::string("Invalid option \"") + upper + "\" in MooseEnum.  Valid options are \"" + _raw_names + "\".");
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

bool MooseEnum::operator!=(int value) const
{
  return value != _current_id;
}

bool
MooseEnum::operator==(unsigned short value) const
{
  return value == _current_id;
}

bool MooseEnum::operator!=(unsigned short value) const
{
  return value != _current_id;
}

bool MooseEnum::operator==(const MooseEnum & value) const
{
  return value._current_name == _current_name;
}

bool MooseEnum::operator!=(const MooseEnum & value) const
{
  return value._current_name != _current_name;
}

void
MooseEnum::fillNames(std::string names)
{
  std::vector<std::string> elements;
  // split on commas
  MooseUtils::tokenize(names, elements, 1, ",");

  _names.resize(elements.size());
  int value=0;
  for (unsigned int i=0; i<elements.size(); ++i)
  {
    std::vector<std::string> name_value;
    // split on equals sign
    MooseUtils::tokenize(MooseUtils::trim(elements[i]), name_value, 1, "=");

    // See if there is a value supplied for this option
    mooseAssert(name_value.size() <= 2, "Invalid option supplied in MooseEnum");
    if (name_value.size() == 2)
    {
      std::istringstream iss(name_value[1]);
      iss >> value;
    }

    name_value[0] = MooseUtils::trim(name_value[0]);

    // preserve case for raw options, append to list
    if (i)
    {
      _raw_names += ", ";
      _raw_names_no_commas += " ";
    }
    _raw_names += name_value[0];
    _raw_names_no_commas += name_value[0];

    // convert name to uppercase
    std::string upper(name_value[0]);
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    // populate internal datastructures
    _names[i] = upper;
    _name_to_id[upper] = value++;
  }
}
