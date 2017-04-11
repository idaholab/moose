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

#include "MooseEnumBase.h"
#include "MooseUtils.h"
#include "MooseError.h"

#include <sstream>
#include <algorithm>
#include <iterator>
#include <limits>
#include <string>
#include <iostream>

const int MooseEnumBase::INVALID_ID = std::numeric_limits<int>::min();

MooseEnumBase::MooseEnumBase(std::string names, bool allow_out_of_range)
  : _out_of_range_index(allow_out_of_range ? INVALID_ID + 1 : 0)
{
  if (names.find(',') != std::string::npos)
  {
    mooseDeprecated("Please use a space to separate options in a MooseEnum, commas are "
                    "deprecated\nMooseEnum initialized with names: \"",
                    names,
                    '\"');
    fillNames(names, ",");
  }
  else
    fillNames(names);
}

MooseEnumBase::MooseEnumBase(const MooseEnumBase & other_enum)
  : _names(other_enum._names),
    _raw_names(other_enum._raw_names),
    _name_to_id(other_enum._name_to_id),
    _deprecated_names(other_enum._deprecated_names),
    _out_of_range_index(other_enum._out_of_range_index)
{
}

/**
 * Private constuctor for use by libmesh::Parameters
 */
MooseEnumBase::MooseEnumBase() {}

void
MooseEnumBase::deprecate(const std::string & name, const std::string & new_name)
{
  std::string upper(name);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  std::string upper_new(new_name);
  std::transform(upper_new.begin(), upper_new.end(), upper_new.begin(), ::toupper);

  _deprecated_names[upper] = upper_new;

  checkDeprecated();
}

void
MooseEnumBase::fillNames(std::string names, std::string option_delim)
{
  std::vector<std::string> elements;
  MooseUtils::tokenize(names, elements, 1, option_delim);
  for (const std::string & raw_name : elements)
    addEnumerationName(raw_name);
}

void
MooseEnumBase::addEnumerationNames(const std::string & names)
{
  fillNames(names);
}

void
MooseEnumBase::removeEnumerationNames(const std::string & names)
{
  std::vector<std::string> elements;
  MooseUtils::tokenize(names, elements, 1, " ");
  for (const std::string & raw_name : elements)
    removeEnumerationName(raw_name);
}

void
MooseEnumBase::checkDeprecatedBase(const std::string & name_upper) const
{
  std::map<std::string, std::string>::const_iterator it = _deprecated_names.find(name_upper);

  if (it != _deprecated_names.end())
  {
    if (it->second != "")
      mooseWarning(name_upper + " is deprecated, consider using " + it->second);
    else
      mooseWarning(name_upper + " is deprecated");
  }
}

void
MooseEnumBase::addEnumerationName(const std::string & raw_name)
{
  // Make sure the option is not malformed
  if (raw_name.find_first_of('=') == 0 ||
      raw_name.find_last_of('=') == raw_name.length() - 1)
    mooseError("You cannot place whitespace around the '=' character in MooseEnumBase");

  // Split on equals sign
  std::vector<std::string> name_value;
  MooseUtils::tokenize(MooseUtils::trim(raw_name), name_value, 1, "=");

  // There should be one or two items in the name_value
  if (name_value.size() < 1 || name_value.size() > 2)
    mooseError("Invalid option supplied in MooseEnumBase: ", raw_name);

  // Remove un-wanted space around string
  name_value[0] = MooseUtils::trim(name_value[0]);

  // preserve case for raw options, append to list
  if (!_raw_names.empty())
    _raw_names += " ";
  _raw_names += name_value[0];

  // See if there is a value supplied for this option
  // strtol allows for proper conversions of both int and hex strings
  int value;
  if (name_value.size() == 2)
    value = strtol(name_value[1].c_str(), NULL, 0);
  else if (_ids.empty())
    value = 0;
  else
    value = *_ids.rbegin() + 1;

  // convert name to uppercase
  std::string upper(name_value[0]);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  // check ids and names
  if (_ids.find(value) != _ids.end())
    mooseError("The id ", value, " already exists in the enumeration.");
  if (_name_to_id.find(upper) != _name_to_id.end())
    mooseError("The name ", upper, " already exists in the enumeration.");

  // populate internal datastructures
  _names.push_back(upper);
  _ids.insert(value);
  _name_to_id[upper] = value;
  _name_to_raw_name[upper] = name_value[0] + " "; // add space to make the name unique for removal
}

int
MooseEnumBase::id(std::string name) const
{
  std::transform(name.begin(), name.end(), name.begin(), ::toupper);
  std::map<std::string, int>::const_iterator iter = _name_to_id.find(name);
  if (iter == _name_to_id.end())
    mooseError("The name ", name, " is not a possible enumeration value.");
  return iter->second;
}

void
MooseEnumBase::removeEnumerationName(std::string name)
{
  std::transform(name.begin(), name.end(), name.begin(), ::toupper);
  std::vector<std::string>::const_iterator iter = std::find(_names.begin(), _names.end(), name);
  if (iter == _names.end())
    mooseError("The name ", name, " is not a possible enumeration value, thus can not be removed.");
  else
  {
    _ids.erase(_ids.find(_name_to_id[name]));
    _name_to_id.erase(_name_to_id.find(name));
    _names.erase(iter);
    _raw_names.erase(_raw_names.find(_name_to_raw_name[name]), name.size() + 1);
    _name_to_raw_name.erase(_name_to_raw_name.find(name));
  }
}
