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
#include "Moose.h"

#include <sstream>
#include <algorithm>
#include <iterator>


MooseEnum::MooseEnum(std::string names) :
    _raw_names(names),
    _current_id(-1)
{
  fillNames(names);
  buildNameToIDMap();
}

MooseEnum::MooseEnum(std::string names, std::string default_name) :
    _raw_names(names)
{
  fillNames(names);
  buildNameToIDMap();

  std::string upper(default_name);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
  *this = upper;
}

/**
 * Private constuctor for use by libmesh::Parameters
 */
MooseEnum::MooseEnum() :
    _current_id(-2)
{
}

MooseEnum &
MooseEnum::operator=(const std::string & name)
{
  std::string upper(name);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  if (std::find(_names.begin(), _names.end(), upper) == _names.end())
    mooseError(std::string("Invalid option \"") + upper + "\" in MooseEnum.  Valid options are \"" + _raw_names << "\".");

  _current_name = upper;
  _current_id = _name_to_id[upper];
  return *this;
}

bool
MooseEnum::operator==(const char * name) const
{
  std::string upper(name);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  return _current_name == upper;
}

void
MooseEnum::fillNames(std::string names)
{
  // Case preserving/Case insensitive
  _raw_names = names;

  std::string upper(names);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  std::istringstream iss(upper);
  std::copy(std::istream_iterator<std::string>(iss),
            std::istream_iterator<std::string>(),
            std::back_inserter<std::vector<std::string> >(_names));
}

void
MooseEnum::buildNameToIDMap()
{
  for(unsigned int i=0; i<_names.size(); i++)
    _name_to_id[_names[i]] = i;
}
