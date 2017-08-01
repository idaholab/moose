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
#include "Conversion.h"

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
  : _items(other_enum._items),
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
  std::string upper(MooseUtils::toUpper(name));
  std::string upper_new(MooseUtils::toUpper(new_name));
  _deprecated_names[upper] = upper_new;
  checkDeprecated();
}

void
MooseEnumBase::fillNames(std::string names, std::string option_delim)
{
  std::vector<std::string> elements;
  // split on spaces
  MooseUtils::tokenize(names, elements, 1, option_delim);

  int value = 0;
  for (unsigned int i = 0; i < elements.size(); ++i)
  {
    std::vector<std::string> name_value;

    // Make sure the option is not malformed
    if (elements[i].find_first_of('=') == 0 ||
        elements[i].find_last_of('=') == elements[i].length() - 1)
      mooseError("You cannot place whitespace around the '=' character in MooseEnumBase");

    // split on equals sign
    MooseUtils::tokenize(MooseUtils::trim(elements[i]), name_value, 1, "=");

    if (name_value.size() < 1 || name_value.size() > 2)
      mooseError("Invalid option supplied in MooseEnumBase: ", elements[i]);

    // See if there is a value supplied for this option
    // strtol allows for proper conversions of both int and hex strings
    if (name_value.size() == 2)
      value = strtol(name_value[1].c_str(), NULL, 0);

    // create item entry
    _items.emplace(name_value[0], value++);
  }
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

std::vector<std::string>
MooseEnumBase::getNames() const
{
  std::vector<std::string> out;
  out.reserve(_items.size());
  for (auto it = _items.begin(); it != _items.end(); ++it)
    out.push_back(it->name());
  return out;
}

std::string
MooseEnumBase::getRawNames() const
{
  return Moose::stringify(_items, " ");
}

std::set<MooseEnumItem>::const_iterator
MooseEnumBase::find(const std::string & name) const
{
  return std::find_if(_items.begin(), _items.end(), [&name](MooseEnumItem const & item) {
    return item.name() == name;
  });
}

std::set<MooseEnumItem>::const_iterator
MooseEnumBase::find(int id) const
{
  return std::find_if(
      _items.begin(), _items.end(), [&id](MooseEnumItem const & item) { return item.id() == id; });
}

std::set<MooseEnumItem>::const_iterator
MooseEnumBase::find(const MooseEnumItem & other) const
{
  return std::find_if(
      _items.begin(), _items.end(), [&other](MooseEnumItem const & item) { return item == other; });
}
