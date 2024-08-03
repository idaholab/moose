//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseEnumBase.h"
#include "MultiMooseEnum.h"
#include "MooseUtils.h"
#include "MooseError.h"
#include "Conversion.h"

#include <initializer_list>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <limits>
#include <string>
#include <iostream>

MultiMooseEnum::MultiMooseEnum(std::string valid_names,
                               std::string initialization_values,
                               bool allow_out_of_range)
  : MooseEnumBase(valid_names, allow_out_of_range)
{
  *this = initialization_values;
}

MultiMooseEnum::MultiMooseEnum(std::string valid_names,
                               const char * initialization_values,
                               bool allow_out_of_range)
  : MultiMooseEnum(valid_names, std::string(initialization_values), allow_out_of_range)
{
}

MultiMooseEnum::MultiMooseEnum(std::string valid_names, bool allow_out_of_range)
  // Set default variable value to nothing ("")
  : MultiMooseEnum(valid_names, "", allow_out_of_range)
{
}

MultiMooseEnum::MultiMooseEnum(const MultiMooseEnum & other_enum)
  : MooseEnumBase(other_enum), _current_values(other_enum._current_values)
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
  return isValueSet(value);
}

bool
MultiMooseEnum::operator!=(const MultiMooseEnum & value) const
{
  return !(*this == value);
}

bool
MultiMooseEnum::isValueSet(const std::string & value) const
{
  return std::find_if(_current_values.begin(),
                      _current_values.end(),
                      [&value](const MooseEnumItem & item)
                      { return item == value; }) != _current_values.end();
}

bool
MultiMooseEnum::isValueSet(int value) const
{
  return std::find_if(_current_values.begin(),
                      _current_values.end(),
                      [&value](const MooseEnumItem & item)
                      { return item == value; }) != _current_values.end();
}

bool
MultiMooseEnum::isValueSet(unsigned short value) const
{
  return std::find_if(_current_values.begin(),
                      _current_values.end(),
                      [&value](const MooseEnumItem & item)
                      { return item == value; }) != _current_values.end();
}

bool
MultiMooseEnum::isValueSet(const MultiMooseEnum & value) const
{
  for (const auto & item : value._current_values)
    if (!isValueSet(item))
      return false;
  return true;
}

bool
MultiMooseEnum::isValueSet(const MooseEnumItem & value) const
{
  return std::find_if(_current_values.begin(),
                      _current_values.end(),
                      [&value](const MooseEnumItem & item)
                      { return item == value; }) != _current_values.end();
}

MultiMooseEnum &
MultiMooseEnum::operator=(const std::string & names)
{
  std::vector<std::string> names_vector;
  MooseUtils::tokenize(names, names_vector, 1, " ");
  return assignValues(names_vector.begin(), names_vector.end(), false);
}

MultiMooseEnum &
MultiMooseEnum::operator=(const std::vector<std::string> & names)
{
  return assignValues(names.begin(), names.end(), false);
}

MultiMooseEnum &
MultiMooseEnum::operator=(const std::set<std::string> & names)
{
  return assignValues(names.begin(), names.end(), false);
}

void
MultiMooseEnum::eraseSetValue(const std::string & names)
{
  std::vector<std::string> names_vector;
  MooseUtils::tokenize(names, names_vector, 1, " ");
  removeSetValues(names_vector.begin(), names_vector.end());
}

void
MultiMooseEnum::erase(const std::string & names)
{
  mooseDeprecated("MultiMooseEnum::erase is deprecated, use MultiMooseEnum::eraseSetValue");
  MultiMooseEnum::eraseSetValue(names);
}

void
MultiMooseEnum::eraseSetValue(const std::vector<std::string> & names)
{
  removeSetValues(names.begin(), names.end());
}

void
MultiMooseEnum::erase(const std::vector<std::string> & names)
{
  mooseDeprecated("MultiMooseEnum::erase is deprecated, use MultiMooseEnum::eraseSetValue");
  MultiMooseEnum::eraseSetValue(names);
}

void
MultiMooseEnum::eraseSetValue(const std::set<std::string> & names)
{
  removeSetValues(names.begin(), names.end());
}

void
MultiMooseEnum::erase(const std::set<std::string> & names)
{
  mooseDeprecated("MultiMooseEnum::erase is deprecated, use MultiMooseEnum::eraseSetValue");
  MultiMooseEnum::eraseSetValue(names);
}

void
MultiMooseEnum::setAdditionalValue(const std::string & names)
{
  std::vector<std::string> names_vector;
  MooseUtils::tokenize(names, names_vector, 1, " ");
  assignValues(names_vector.begin(), names_vector.end(), true);
}

void
MultiMooseEnum::push_back(const std::string & names)
{
  mooseDeprecated(
      "MultiMooseEnum::push_back is deprecated, use MultiMooseEnum::setAdditionalValue");
  MultiMooseEnum::setAdditionalValue(names);
}

void
MultiMooseEnum::setAdditionalValue(const std::vector<std::string> & names)
{
  assignValues(names.begin(), names.end(), true);
}

void
MultiMooseEnum::push_back(const std::vector<std::string> & names)
{
  mooseDeprecated(
      "MultiMooseEnum::push_back is deprecated, use MultiMooseEnum::setAdditionalValue");
  MultiMooseEnum::setAdditionalValue(names);
}

void
MultiMooseEnum::setAdditionalValue(const std::set<std::string> & names)
{
  assignValues(names.begin(), names.end(), true);
}

void
MultiMooseEnum::push_back(const std::set<std::string> & names)
{
  mooseDeprecated(
      "MultiMooseEnum::push_back is deprecated, use MultiMooseEnum::setAdditionalValue");
  MultiMooseEnum::setAdditionalValue(names);
}

void
MultiMooseEnum::setAdditionalValue(const MultiMooseEnum & other_enum)
{
  assignValues(other_enum.begin(), other_enum.end(), true);
}

void
MultiMooseEnum::push_back(const MultiMooseEnum & other_enum)
{
  mooseDeprecated(
      "MultiMooseEnum::push_back is deprecated, use MultiMooseEnum::setAdditionalValue");
  MultiMooseEnum::setAdditionalValue(other_enum);
}

const std::string &
MultiMooseEnum::operator[](unsigned int i) const
{
  mooseAssert(i < _current_values.size(),
              "Access out of bounds in MultiMooseEnum (i: " << i << " size: "
                                                            << _current_values.size() << ")");

  return _current_values[i].rawName();
}

unsigned int
MultiMooseEnum::get(unsigned int i) const
{
  mooseAssert(i < _current_values.size(),
              "Access out of bounds in MultiMooseEnum (i: " << i << " size: "
                                                            << _current_values.size() << ")");

  return _current_values[i].id();
}

template <typename InputIterator>
MultiMooseEnum &
MultiMooseEnum::assignValues(InputIterator first, InputIterator last, bool append)
{
  if (!append)
    clearSetValues();

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
        _current_values.push_back(created);
      }
    }
    else
      _current_values.push_back(*iter);
  }
  checkDeprecated();
  return *this;
}

template <typename InputIterator>
void
MultiMooseEnum::removeSetValues(InputIterator first, InputIterator last)
{
  // Create a new list of enumerations by striping out the supplied values
  for (InputIterator it = first; it != last; ++it)
  {
    std::vector<MooseEnumItem>::iterator iter =
        std::find_if(_current_values.begin(),
                     _current_values.end(),
                     [it](const MooseEnumItem & item) { return item == *it; });
    if (iter != _current_values.end())
      _current_values.erase(iter);
  }
}

void
MultiMooseEnum::clearSetValues()
{
  _current_values.clear();
}

void
MultiMooseEnum::clear()
{
  mooseDeprecated("MultiMooseEnum::clear is deprecated, use MultiMooseEnum::clearSetValues");
  clearSetValues();
}

unsigned int
MultiMooseEnum::size() const
{
  return _current_values.size();
}

void
MultiMooseEnum::checkDeprecated() const
{
  for (const auto & item : _current_values)
    MooseEnumBase::checkDeprecated(item);
}

std::ostream &
operator<<(std::ostream & out, const MultiMooseEnum & obj)
{
  out << Moose::stringify(obj._current_values, " ");
  return out;
}

void
MultiMooseEnum::addValidName(const std::string & name)
{
  addEnumerationName(name);
  checkDeprecated();
}

MooseEnumBase &
MultiMooseEnum::operator+=(const std::string & name)
{
  mooseDeprecated("MultiMooseEnum::operator+= is deprecated, use MultiMooseEnum::addValidName");
  return MooseEnumBase::operator+=(name);
}

void
MultiMooseEnum::addValidName(const std::initializer_list<std::string> & names)
{
  for (const auto & name : names)
    addValidName(name);
}

MooseEnumBase &
MultiMooseEnum::operator+=(const std::initializer_list<std::string> & names)
{
  mooseDeprecated("MultiMooseEnum::operator+= is deprecated, use MultiMooseEnum::addValidName");
  return MooseEnumBase::operator+=(names);
}
