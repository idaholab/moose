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
  : MooseEnumBase(other_enum),
    _current_ids(other_enum._current_ids),
    _current_names(other_enum._current_names),
    _current_names_preserved(other_enum._current_names_preserved)
{
}

MultiMooseEnum
MultiMooseEnum::withNamesFrom(const MooseEnumBase & other_enum)
{
  return MultiMooseEnum(other_enum);
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
  for (const auto & me : value)
    if (!contains(me))
      return false;

  // If you get here, they must be the same
  return true;
}

bool
MultiMooseEnum::operator!=(const MultiMooseEnum & value) const
{
  return !(*this == value);
}

bool
MultiMooseEnum::contains(const std::string & value) const
{
  std::string upper(value);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  return std::find(_current_names.begin(), _current_names.end(), upper) != _current_names.end();
}

bool
MultiMooseEnum::contains(int value) const
{
  return std::find(_current_ids.begin(), _current_ids.end(), value) != _current_ids.end();
}

bool
MultiMooseEnum::contains(unsigned short value) const
{
  return std::find(_current_ids.begin(), _current_ids.end(), value) != _current_ids.end();
}

bool
MultiMooseEnum::contains(const MultiMooseEnum & value) const
{
  std::set<int> lookup_set(_current_ids.begin(), _current_ids.end());

  for (const auto & id : value._current_ids)
    if (lookup_set.find(id) == lookup_set.end())
      return false;

  return true;
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
  mooseAssert(i < _current_names_preserved.size(),
              "Access out of bounds in MultiMooseEnum (i: " << i << " size: "
                                                            << _current_names_preserved.size()
                                                            << ")");

  return _current_names_preserved[i];
}

unsigned int
MultiMooseEnum::get(unsigned int i) const
{
  mooseAssert(i < _current_ids.size(),
              "Access out of bounds in MultiMooseEnum (i: " << i << " size: " << _current_ids.size()
                                                            << ")");

  return _current_ids[i];
}

template <typename InputIterator>
MultiMooseEnum &
MultiMooseEnum::assign(InputIterator first, InputIterator last, bool append)
{
  if (!append)
    clear();

  std::copy(first, last, std::back_inserter(_current_names_preserved));
  for (InputIterator it = first; it != last; ++it)
  {
    std::string upper(*it);
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    checkDeprecatedBase(upper);

    _current_names.insert(upper);

    if (std::find(_names.begin(), _names.end(), upper) == _names.end())
    {
      if (_out_of_range_index == 0) // Are out of range values allowed?
        mooseError("Invalid option \"",
                   upper,
                   "\" in MultiMooseEnum.  Valid options (not case-sensitive) are \"",
                   _raw_names,
                   "\".");
      else
      {
        // Allow values assigned outside of the enumeration range
        _names.push_back(upper);

        int current_id = _out_of_range_index++;
        _name_to_id[upper] = current_id;

        _current_ids.push_back(current_id);
      }
    }
    else
      _current_ids.push_back(_name_to_id[upper]);
  }
  return *this;
}

template <typename InputIterator>
void
MultiMooseEnum::remove(InputIterator first, InputIterator last)
{
  // Create a new list of enumerations by striping out the supplied values
  std::set<std::string> current = _current_names;
  for (InputIterator it = first; it != last; ++it)
  {
    // Values stored as upper case
    std::string upper(*it);
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    std::set<std::string>::iterator found = current.find(upper);
    if (found != current.end())
      current.erase(found);
  }

  // Build the new enumeration
  assign(current.begin(), current.end(), false);
}

void
MultiMooseEnum::clear()
{
  _current_names.clear();
  _current_names_preserved.clear();
  _current_ids.clear();
}

unsigned int
MultiMooseEnum::size() const
{
  mooseAssert(_current_ids.size() == _current_names_preserved.size(),
              "Internal inconsistency between id and name vectors in MultiMooseEnum");
  return _current_ids.size();
}

unsigned int
MultiMooseEnum::unique_items_size() const
{
  std::set<int> unique_ids(_current_ids.begin(), _current_ids.end());
  return unique_ids.size();
}

void
MultiMooseEnum::checkDeprecated() const
{
  for (const auto & name : _current_names)
    checkDeprecatedBase(name);
}

std::ostream &
operator<<(std::ostream & out, const MultiMooseEnum & obj)
{
  std::copy(obj._current_names.begin(),
            obj._current_names.end(),
            infix_ostream_iterator<std::string>(out, " "));
  return out;
}

void
MultiMooseEnum::removeEnumerationName(std::string name)
{
  std::transform(name.begin(), name.end(), name.begin(), ::toupper);
  if (_current_names.find(name) != _current_names.end())
    mooseError("A current enumeration value of ", name, " cannot be removed.");
  MooseEnumBase::removeEnumerationName(name);
}
