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

#include <sstream>
#include <algorithm>
#include <iterator>
#include <limits>
#include <string>
#include <iostream>

MultiMooseEnum::MultiMooseEnum(std::string names, std::string default_names, bool allow_out_of_range) :
    MooseEnumBase(names, allow_out_of_range)
{
  *this = default_names;
}

MultiMooseEnum::MultiMooseEnum(const MultiMooseEnum & other_enum) :
    MooseEnumBase(other_enum),
    _current_ids(other_enum._current_ids),
    _current_names(other_enum._current_names),
    _current_names_preserved(other_enum._current_names_preserved)
{
}

/**
 * Private constuctor for use by libmesh::Parameters
 */
MultiMooseEnum::MultiMooseEnum()
{
}

MultiMooseEnum &
MultiMooseEnum::operator=(const std::string & names)
{
  std::vector<std::string> names_vector;
  MooseUtils::tokenize(names, names_vector, 1, " ");
  return *this = names_vector;
}

MultiMooseEnum &
MultiMooseEnum::operator=(const std::vector<std::string> & names)
{
  std::set<std::string> names_set(names.begin(), names.end());
  return *this = names_set;
}

MultiMooseEnum &
MultiMooseEnum::operator=(const std::set<std::string> & names)
{
  return assign(names, false);
}

void
MultiMooseEnum::insert(const std::string & names)
{
  std::vector<std::string> names_vector;
  MooseUtils::tokenize(names, names_vector, 1, " ");
  insert(names_vector);
}

void
MultiMooseEnum::insert(const std::vector<std::string> & names)
{
  std::set<std::string> names_set(names.begin(), names.end());
  insert(names_set);
}

void
MultiMooseEnum::insert(const std::set<std::string> & names)
{
  assign(names, true);
}

MultiMooseEnum &
MultiMooseEnum::assign(const std::set<std::string> & names, bool append)
{
  if (!append)
    clear();

  _current_names_preserved.insert(names.begin(), names.end());
  for (std::set<std::string>::const_iterator it = names.begin(); it != names.end(); ++it)
  {
    std::string upper(*it);
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    _current_names.insert(upper);

    if (std::find(_names.begin(), _names.end(), upper) == _names.end())
    {
      if (_out_of_range_index == 0)     // Are out of range values allowed?
        mooseError(std::string("Invalid option \"") + upper + "\" in MultiMooseEnum.  Valid options (not case-sensitive) are \"" + _raw_names + "\".");
      else
      {
        // Allow values assigned outside of the enumeration range
        _names.push_back(upper);

        int current_id = _out_of_range_index++;
        _name_to_id[upper] = current_id;

        _current_ids.insert(current_id);
      }
    }
    else
      _current_ids.insert(_name_to_id[upper]);
  }

  return *this;
}

bool
MultiMooseEnum::contains(const std::string & value) const
{
  std::string upper(value);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  return _current_names.find(upper) != _current_names.end();
}

bool
MultiMooseEnum::contains(int value) const
{
  return _current_ids.find(value) != _current_ids.end();
}

bool
MultiMooseEnum::contains(unsigned short value) const
{
  return _current_ids.find(value) != _current_ids.end();
}

bool
MultiMooseEnum::operator==(const MultiMooseEnum & value) const
{
  return std::equal(value._current_ids.begin(), value._current_ids.end(),
                    _current_ids.begin());
}

void
MultiMooseEnum::clear()
{
  _current_names.clear();
  _current_names_preserved.clear();
  _current_ids.clear();
}

std::ostream &
operator<<(std::ostream & out, const MultiMooseEnum & obj)
{
  std::copy(obj._current_names_preserved.begin(), obj._current_names_preserved.end(), std::ostream_iterator<std::string>(out, " "));
  return out;
}
