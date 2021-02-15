//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseEnum.h"
#include "MooseUtils.h"
#include "MooseError.h"
#include "Conversion.h"

#include <sstream>
#include <algorithm>
#include <iterator>
#include <limits>
#include <string>
#include <iostream>

MooseEnum::MooseEnum(std::string names, std::string default_name, bool allow_out_of_range)
  : MooseEnumBase(names, allow_out_of_range), _current("", MooseEnumItem::INVALID_ID)
{
  *this = default_name;
}

MooseEnum::MooseEnum(const MooseEnum & other_enum)
  : MooseEnumBase(other_enum), _current(other_enum._current)
{
}

/**
 * Private constuctor for use by libmesh::Parameters
 */
MooseEnum::MooseEnum() : _current("", MooseEnumItem::INVALID_ID) {}

MooseEnum &
MooseEnum::operator=(const std::string & name)
{
  assign(name);
  return *this;
}

MooseEnum &
MooseEnum::operator=(int value)
{
  assign(value);
  return *this;
}

MooseEnum &
MooseEnum::operator=(const MooseEnumItem & item)
{
  assign(item);
  return *this;
}

void
MooseEnum::assign(const std::string & name)
{
  if (name == "")
  {
    _current = MooseEnumItem("", MooseEnumItem::INVALID_ID);
    return;
  }

  std::set<MooseEnumItem>::const_iterator iter = find(name);
  if (iter == _items.end())
  {
    if (!_allow_out_of_range) // Are out of range values allowed?
      mooseError("Invalid option \"",
                 name,
                 "\" in MooseEnum.  Valid options (not case-sensitive) are \"",
                 getRawNames(),
                 "\".");
    else
    {
      _current = MooseEnumItem(name, getNextValidID());
      _items.insert(_current);
    }
  }
  else
    _current = *iter;

  checkDeprecated();
}

void
MooseEnum::assign(int value)
{
  if (value == MooseEnumItem::INVALID_ID)
  {
    _current = MooseEnumItem("", MooseEnumItem::INVALID_ID);
    return;
  }

  std::set<MooseEnumItem>::const_iterator iter = find(value);
  if (iter == _items.end())
    mooseError("Invalid id \"",
               value,
               "\" in MooseEnum. Valid ids are \"",
               Moose::stringify(getIDs()),
               "\".");
  else
    _current = *iter;

  checkDeprecated();
}

void
MooseEnum::assign(const MooseEnumItem & item)
{
  std::set<MooseEnumItem>::const_iterator iter = find(item);
  if (iter == _items.end())
    mooseError("Invalid item \"",
               item,
               "\" in MooseEnum. Valid ids are \"",
               Moose::stringify(items()),
               "\".");
  else
    _current = *iter;

  checkDeprecated();
}

bool
MooseEnum::operator==(const char * name) const
{
  std::string upper(MooseUtils::toUpper(name));

  mooseAssert(_allow_out_of_range || find(upper) != _items.end(),
              "Invalid string comparison \"" + upper +
                  "\" in MooseEnum.  Valid options (not case-sensitive) are \"" + getRawNames() +
                  "\".");

  return _current == upper;
}

bool
MooseEnum::operator!=(const char * name) const
{
  return !(*this == name);
}

bool
MooseEnum::operator==(int value) const
{
  return value == _current;
}

bool
MooseEnum::operator!=(int value) const
{
  return value != _current;
}

bool
MooseEnum::operator==(unsigned short value) const
{
  return value == _current;
}

bool
MooseEnum::operator!=(unsigned short value) const
{
  return value != _current;
}

bool
MooseEnum::compareCurrent(const MooseEnum & other, CompareMode mode) const
{
  switch (mode)
  {
    case CompareMode::COMPARE_BOTH:
      return (_current.id() == other._current.id()) && (_current.name() == other._current.name());
    case CompareMode::COMPARE_NAME:
      return _current.name() == other._current.name();
    case CompareMode::COMPARE_ID:
      return _current.id() == other._current.id();
  }
  return false;
}

bool
MooseEnum::operator==(const MooseEnum & value) const
{
  mooseDeprecated("This method will be removed because the meaning is not well defined, please use "
                  "the 'compareCurrent' method instead.");
  return value._current.name() == _current.name();
}

bool
MooseEnum::operator!=(const MooseEnum & value) const
{
  mooseDeprecated("This method will be removed because the meaning is not well defined, please use "
                  "the 'compareCurrent' method instead.");
  return value._current.name() != _current.name();
}

void
MooseEnum::checkDeprecated() const
{
  MooseEnumBase::checkDeprecated(_current);
}
