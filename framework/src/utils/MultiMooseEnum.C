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

#include "MultiMooseEnum.h"
#include "MooseUtils.h"
#include "MooseError.h"
#include "InfixIterator.h"
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

  // Return false if this enum does not contain an item from the other
  return (_current == value._current) && (_items == value._items);
}

bool
MultiMooseEnum::operator!=(const MultiMooseEnum & value) const
{
  return !(*this == value);
}

bool
MultiMooseEnum::contains(const std::string & value) const
{
  std::string upper(MooseUtils::toUpper(value));
  return std::find_if(_current.begin(), _current.end(), [&upper](const MooseEnumItem & item) {
           return item.name() == upper;
         }) != _current.end();
}

bool
MultiMooseEnum::contains(int value) const
{
  return std::find_if(_current.begin(), _current.end(), [&value](const MooseEnumItem & item) {
           return item.id() == value;
         }) != _current.end();
}

bool
MultiMooseEnum::contains(unsigned short value) const
{
  return std::find_if(_current.begin(), _current.end(), [&value](const MooseEnumItem & item) {
           return item.id() == value;
         }) != _current.end();
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
  return std::find_if(_current.begin(), _current.end(), [&value](const MooseEnumItem & item) {
           return item.id() == value.id();
         }) != _current.end();
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

const std::string & MultiMooseEnum::operator[](unsigned int i) const
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
    std::string upper(MooseUtils::toUpper(*it));
    checkDeprecatedBase(upper);
    const auto iter = find(upper);

    if (iter == _items.end())
    {
      if (_out_of_range_index == 0) // Are out of range values allowed?
        mooseError("Invalid option \"",
                   upper,
                   "\" in MultiMooseEnum.  Valid options (not case-sensitive) are \"",
                   getRawNames(),
                   "\".");
      else
      {
        MooseEnumItem created(upper, _out_of_range_index++);
        _current.push_back(created);
        _items.insert(created);
      }
    }
    else
      _current.push_back(*iter);
  }
  return *this;
}

template <typename InputIterator>
void
MultiMooseEnum::remove(InputIterator first, InputIterator last)
{
  // Create a new list of enumerations by striping out the supplied values
  for (InputIterator it = first; it != last; ++it)
  {
    // Values stored as upper case
    std::string upper(MooseUtils::toUpper(*it));
    std::vector<MooseEnumItem>::iterator iter =
        std::find_if(_current.begin(), _current.end(), [&upper](const MooseEnumItem & item) {
          return item.name() == upper;
        });
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
    checkDeprecatedBase(item.name());
}

std::ostream &
operator<<(std::ostream & out, const MultiMooseEnum & obj)
{
  out << Moose::stringify(obj._current, " ");
  return out;
}
