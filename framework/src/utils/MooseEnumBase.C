//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
#include <cstdlib>

MooseEnumBase::MooseEnumBase(std::string names, bool allow_out_of_range)
  : _allow_out_of_range(allow_out_of_range)
{
  if (names.find(',') != std::string::npos)
    mooseError("Spaces are required to separate options, comma support has been removed.");
  else
    addEnumerationNames(names);
}

MooseEnumBase::MooseEnumBase(const MooseEnumBase & other_enum)
  : _items(other_enum._items),
    _deprecated_items(other_enum._deprecated_items),
    _allow_out_of_range(other_enum._allow_out_of_range)
{
}

/**
 * Private constuctor for use by libmesh::Parameters
 */
MooseEnumBase::MooseEnumBase() : _allow_out_of_range(false) {}

void
MooseEnumBase::deprecate(const std::string & name, const std::string & raw_name)
{
  std::set<MooseEnumItem>::const_iterator deprecated = find(name);
  if (deprecated == _items.end())
    mooseError("Cannot deprecate the enum item ", name, ", is not an available value.");

  std::set<MooseEnumItem>::const_iterator replaced = find(raw_name);
  if (replaced == _items.end())
    mooseError("Cannot deprecate the enum item ",
               name,
               ", since the replaced item ",
               raw_name,
               " it is not an available value.");

  _deprecated_items.emplace(std::make_pair(*deprecated, *replaced));
  checkDeprecated();
}

void
MooseEnumBase::addEnumerationNames(const std::string & names)
{
  std::vector<std::string> elements;
  MooseUtils::tokenize(names, elements, 1, " ");
  for (const std::string & raw_name : elements)
    addEnumerationName(raw_name);
}

const MooseEnumItem &
MooseEnumBase::addEnumerationName(const std::string & raw_name)
{
  // Make sure the option is not malformed
  if (raw_name.find_first_of('=') == 0 || raw_name.find_last_of('=') == raw_name.length() - 1)
    mooseError("You cannot place whitespace around the '=' character in MooseEnumBase");

  // Split on equals sign
  std::vector<std::string> name_value;
  MooseUtils::tokenize(MooseUtils::trim(raw_name), name_value, 1, "=");

  // There should be one or two items in the name_value
  if (name_value.size() < 1 || name_value.size() > 2)
    mooseError("Invalid option supplied in MooseEnumBase: ", raw_name);

  // Remove un-wanted space around string
  name_value[0] = MooseUtils::trim(name_value[0]);

  // See if there is a value supplied for this option
  // strtol allows for proper conversions of both int and hex strings
  int value;
  if (name_value.size() == 2)
    value = std::strtol(name_value[1].c_str(), NULL, 0);
  else
    value = getNextValidID();

  return addEnumerationName(name_value[0], value);
}

int
MooseEnumBase::getNextValidID() const
{
  int value = -1; // Use -1 so if no values exist the first will be zero
  for (const auto & item : _items)
    value = std::max(value, item.id());
  return ++value;
}

const MooseEnumItem &
MooseEnumBase::addEnumerationName(const std::string & name, const int & value)
{
  return addEnumerationItem(MooseEnumItem(name, value));
}

const MooseEnumItem &
MooseEnumBase::addEnumerationItem(const MooseEnumItem & item)
{
  const auto & item_it = find(item);
  if (item_it != _items.end()) // do nothing for identical insertions
    return *item_it;

  if (find(item.id()) != _items.end())
    mooseError("The supplied id ",
               item.id(),
               " already exists in the enumeration, cannot not add '",
               item,
               "'.");
  if (find(item.name()) != _items.end())
    mooseError("The name '", item.name(), "' already exists in the enumeration.");

  return *_items.insert(item).first;
}

void
MooseEnumBase::checkDeprecated(const MooseEnumItem & item) const
{
  std::map<MooseEnumItem, MooseEnumItem>::const_iterator it = _deprecated_items.find(item);
  if (it != _deprecated_items.end())
  {
    if (it->second.name().empty())
      mooseWarning(item.name() + " is deprecated");
    else
      mooseWarning(item.name() + " is deprecated, consider using " + it->second.name());
  }
}

std::vector<std::string>
MooseEnumBase::getNames() const
{
  std::vector<std::string> out;
  out.reserve(_items.size());
  for (const auto & item : _items)
    out.push_back(item.name());
  return out;
}

std::string
MooseEnumBase::getRawNames() const
{
  return Moose::stringify(_items, " ");
}

std::vector<int>
MooseEnumBase::getIDs() const
{
  std::vector<int> out;
  out.reserve(_items.size());
  for (const auto & item : _items)
    out.push_back(item.id());
  return out;
}

void
MooseEnumBase::addDocumentation(const std::string & name, const std::string & doc)
{
  auto it = find(name);
  if (it == _items.end())
    mooseError("Item '", name, "' not found in addDocumentation.");
  _item_documentation[*it] = doc;
}

const std::map<MooseEnumItem, std::string> &
MooseEnumBase::getItemDocumentation() const
{
  return _item_documentation;
}

std::set<MooseEnumItem>::const_iterator
MooseEnumBase::find(const std::string & name) const
{
  std::string upper = MooseUtils::toUpper(name);
  return std::find_if(_items.begin(),
                      _items.end(),
                      [&upper](MooseEnumItem const & item) { return item.name() == upper; });
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
  const auto upper = MooseUtils::toUpper(other.name());
  return std::find_if(_items.begin(),
                      _items.end(),
                      [&other, &upper](MooseEnumItem const & item)
                      { return item.id() == other.id() && item.name() == upper; });
}

MooseEnumBase &
MooseEnumBase::operator+=(const std::string & name)
{
  addEnumerationName(name);
  checkDeprecated();
  return *this;
}

MooseEnumBase &
MooseEnumBase::operator+=(const std::initializer_list<std::string> & names)
{
  for (const auto & name : names)
    *this += name;
  return *this;
}
