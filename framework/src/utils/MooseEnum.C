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
  : MooseEnumBase(names, allow_out_of_range), _current("", INVALID_ID)
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
MooseEnum::MooseEnum() : _current("", INVALID_ID) {}

MooseEnum &
MooseEnum::operator=(const std::string & name)
{
  if (name == "")
  {
    _current = MooseEnumItem("", INVALID_ID);
    return *this;
  }

  std::string upper(MooseUtils::toUpper(name));
  checkDeprecatedBase(upper);

  std::set<MooseEnumItem>::const_iterator iter = find(upper);
  if (iter == _items.end())
  {
    if (_out_of_range_index == 0) // Are out of range values allowed?
      mooseError(std::string("Invalid option \"") + upper +
                 "\" in MooseEnum.  Valid options (not case-sensitive) are \"" + getRawNames() +
                 "\".");
    else
    {
      _current = MooseEnumItem(name, _out_of_range_index++);
      _items.insert(_current);
    }
  }
  else
    _current = *iter;

  return *this;
}

bool
MooseEnum::operator==(const char * name) const
{
  std::string upper(MooseUtils::toUpper(name));
  std::set<MooseEnumItem>::const_iterator iter = find(upper);

  mooseAssert(_out_of_range_index != 0 || iter != _items.end(),
              std::string("Invalid string comparison \"") + upper +
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
  mooseDeprecated("This method will be removed becuase the meaning is not well defined, please use "
                  "the 'compareCurrent' method instead.");
  return value._current.name() == _current.name();
}

bool
MooseEnum::operator!=(const MooseEnum & value) const
{
  mooseDeprecated("This method will be removed becuase the meaning is not well defined, please use "
                  "the 'compareCurrent' method instead.");
  return value._current.name() != _current.name();
}

void
MooseEnum::checkDeprecated() const
{
  checkDeprecatedBase(_current.name());
}
