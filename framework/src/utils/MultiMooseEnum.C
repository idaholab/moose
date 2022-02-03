//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiMooseEnum.h"
#include "MooseUtils.h"
#include "MooseError.h"
#include "Conversion.h"

#include <sstream>
#include <algorithm>
#include <iterator>
#include <limits>
#include <string>
#include <iostream>

MultiMooseEnum::MultiMooseEnum(std::string names,
                               std::string default_names,
                               bool allow_out_of_range)
  : MooseEnumBase(names, allow_out_of_range)
{
  *this = default_names;
}

MultiMooseEnum::MultiMooseEnum(const MultiMooseEnum & other_enum)
  : MooseEnumBase(other_enum), _current(other_enum._current)
{
}

/**
 * Private constuctor for use by libmesh::Parameters
 */
MultiMooseEnum::MultiMooseEnum() {}

MultiMooseEnum::MultiMooseEnum(const MooseEnumBase & other_enum) : MooseEnumBase(other_enum) {}

bool
MultiMooseEnum::operator==(const MultiMooseEnum & value) const
{
  // Not the same if the lengths are different
  if (value.size() != size())
    return false;

  // Return false if this enum does not contain an item from the other, since they are the same
  // size at this point if this is true then they are equal.
  return contains(value);
}

bool
MultiMooseEnum::operator!=(const MultiMooseEnum & value) const
{
  return !(*this == value);
}

bool
MultiMooseEnum::contains(const std::string & value) const
{
  return std::find_if(_current.begin(),
                      _current.end(),
                      [&value](const MooseEnumItem & item)
                      { return item == value; }) != _current.end();
}

bool
MultiMooseEnum::contains(int value) const
{
  return std::find_if(_current.begin(),
                      _current.end(),
                      [&value](const MooseEnumItem & item)
                      { return item == value; }) != _current.end();
}

bool
MultiMooseEnum::contains(unsigned short value) const
{
  return std::find_if(_current.begin(),
                      _current.end(),
                      [&value](const MooseEnumItem & item)
                      { return item == value; }) != _current.end();
}

bool
MultiMooseEnum::contains(const MultiMooseEnum & value) const
{
  for (const auto & item : value._current)
    if (!contains(item))
      return false;
  return true;
}

bool
MultiMooseEnum::contains(const MooseEnumItem & value) const
{
  return std::find_if(_current.begin(),
                      _current.end(),
                      [&value](const MooseEnumItem & item)
                      { return item == value; }) != _current.end();
}

MultiMooseEnum &
MultiMooseEnum::operator=(const std::string & names)
{
  std::vector<std::string> names_vector;
  MooseUtils::tokenize(names, names_vector, 1, " ");
  return assign(names_vector.begin(), names_vector.end(), false);
}

MultiMooseEnum &
MultiMooseEnum::operator=(const std::vector<std::string> & names)
{
  return assign(names.begin(), names.end(), false);
}

MultiMooseEnum &
MultiMooseEnum::operator=(const std::set<std::string> & names)
{
  return assign(names.begin(), names.end(), false);
}

void
MultiMooseEnum::erase(const std::string & names)
{
  std::vector<std::string> names_vector;
  MooseUtils::tokenize(names, names_vector, 1, " ");
  remove(names_vector.begin(), names_vector.end());
}

void
MultiMooseEnum::erase(const std::vector<std::string> & names)
{
  remove(names.begin(), names.end());
}

void
MultiMooseEnum::erase(const std::set<std::string> & names)
{
  remove(names.begin(), names.end());
}

void
MultiMooseEnum::push_back(const std::string & names)
{
  std::vector<std::string> names_vector;
  MooseUtils::tokenize(names, names_vector, 1, " ");
  assign(names_vector.begin(), names_vector.end(), true);
}

void
MultiMooseEnum::push_back(const std::vector<std::string> & names)
{
  assign(names.begin(), names.end(), true);
}

void
MultiMooseEnum::push_back(const std::set<std::string> & names)
{
  assign(names.begin(), names.end(), true);
}

const std::string &
MultiMooseEnum::operator[](unsigned int i) const
{
  mooseAssert(i < _current.size(),
              "Access out of bounds in MultiMooseEnum (i: " << i << " size: " << _current.size()
                                                            << ")");

  return _current[i].rawName();
}

unsigned int
MultiMooseEnum::get(unsigned int i) const
{
  mooseAssert(i < _current.size(),
              "Access out of bounds in MultiMooseEnum (i: " << i << " size: " << _current.size()
                                                            << ")");

  return _current[i].id();
}

template <typename InputIterator>
MultiMooseEnum &
MultiMooseEnum::assign(InputIterator first, InputIterator last, bool append)
{
  if (!append)
    clear();

  for (InputIterator it = first; it != last; ++it)
  {
    const auto iter = find(*it);
    if (iter == _items.end())
    {
      if (!_allow_out_of_range) // Are out of range values allowed?
        mooseError("Invalid option \"",
                   *it,
                   "\" in MultiMooseEnum.  Valid options (not case-sensitive) are \"",
                   getRawNames(),
                   "\".");
      else
      {
        MooseEnumItem created(*it, getNextValidID());
        addEnumerationItem(created);
        _current.push_back(created);
      }
    }
    else
      _current.push_back(*iter);
  }
  checkDeprecated();
  return *this;
}

template <typename InputIterator>
void
MultiMooseEnum::remove(InputIterator first, InputIterator last)
{
  // Create a new list of enumerations by striping out the supplied values
  for (InputIterator it = first; it != last; ++it)
  {
    std::vector<MooseEnumItem>::iterator iter = std::find_if(
        _current.begin(), _current.end(), [it](const MooseEnumItem & item) { return item == *it; });
    if (iter != _current.end())
      _current.erase(iter);
  }
}

void
MultiMooseEnum::clear()
{
  _current.clear();
}

unsigned int
MultiMooseEnum::size() const
{
  return _current.size();
}

void
MultiMooseEnum::checkDeprecated() const
{
  for (const auto & item : _current)
    MooseEnumBase::checkDeprecated(item);
}

std::ostream &
operator<<(std::ostream & out, const MultiMooseEnum & obj)
{
  out << Moose::stringify(obj._current, " ");
  return out;
}
