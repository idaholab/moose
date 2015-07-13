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

MooseEnumBase::MooseEnumBase(std::string names, bool allow_out_of_range) :
    _out_of_range_index(allow_out_of_range ? INVALID_ID + 1 : 0)
{
  if (names.find(',') != std::string::npos)
  {
    mooseDeprecated("Please use a space to separate options in a MooseEnum, commas are deprecated\nMooseEnum initialized with names: \"" << names << '\"');
    fillNames(names, ",");
  }
  else
    fillNames(names);
}

MooseEnumBase::MooseEnumBase(const MooseEnumBase & other_enum) :
    _names(other_enum._names),
    _raw_names(other_enum._raw_names),
    _name_to_id(other_enum._name_to_id),
    _out_of_range_index(other_enum._out_of_range_index)
{
}

/**
 * Private constuctor for use by libmesh::Parameters
 */
MooseEnumBase::MooseEnumBase()
{
}

MooseEnumBase::~MooseEnumBase()
{
}

void
MooseEnumBase::deprecate(const std::string & name, const std::string & new_name)
{
  std::string formal_name = formalize(name);
  if (std::find(_names.begin(), _names.end(), formal_name) == _names.end())
    mooseError("trying to deprecate a invalid name");

  _deprecated_names[formal_name] = formalize(new_name);

  // because this function is separated from the constructors, we need to check
  // if the current name is deprecated or not.
  checkDeprecatedCurrent();
}

std::set<std::string>
MooseEnumBase::getDeprecatedNames() const
{
  std::set<std::string> names;
  std::map<std::string, std::string>::const_iterator it = _deprecated_names.begin();
  for (; it != _deprecated_names.end(); ++it)
    names.insert(it->first);
  return names;
}

bool
MooseEnumBase::isDeprecated(const std::string & name) const
{
  std::string upper(name);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
  return (_deprecated_names.find(upper) != _deprecated_names.end());
}

void
MooseEnumBase::fillNames(std::string names, std::string option_delim)
{
  std::vector<std::string> elements;
  // split on spaces
  MooseUtils::tokenize(names, elements, 1, option_delim);

  _names.resize(elements.size());
  int value=0;
  for (unsigned int i=0; i<elements.size(); ++i)
  {
    std::vector<std::string> name_value;

    // Make sure the option is not malformed
    if (elements[i].find_first_of('=') == 0 || elements[i].find_last_of('=') == elements[i].length()-1)
      mooseError("You cannot place whitespace around the '=' character in MooseEnumBase");

    // split on equals sign
    MooseUtils::tokenize(MooseUtils::trim(elements[i]), name_value, 1, "=");

    if (name_value.size() < 1 || name_value.size() > 2)
      mooseError("Invalid option supplied in MooseEnumBase: " << elements[i]);

    // See if there is a value supplied for this option
    // strtol allows for proper conversions of both int and hex strings
    if (name_value.size() == 2)
      value = strtol(name_value[1].c_str(), NULL, 0);

    name_value[0] = MooseUtils::trim(name_value[0]);

    // preserve case for raw options, append to list
    if (i)
      _raw_names += " ";
    _raw_names += name_value[0];

    // populate internal datastructures
    _names[i] = formalize(name_value[0]);
    _name_to_id[_names[i]] = value++;
  }
}

std::string
MooseEnumBase::formalize(const char * name) const
{
  std::string upper(name);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
  std::map<std::string, std::string>::const_iterator it = _deprecated_names.find(upper);
  if (it != _deprecated_names.end())
  {
    if (it->second != "")
    {
      mooseWarning(upper+" is deprecated, consider using "+it->second);
      upper = it->second;
    }
    else
      mooseWarning(upper+" is deprecated");
  }
  return upper;
}

std::string
MooseEnumBase::formalize(const std::string & name) const
{
  return formalize(name.c_str());
}
