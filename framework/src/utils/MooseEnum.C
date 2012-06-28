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
  *this = default_name;
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
  if (std::find(_names.begin(), _names.end(), name) == _names.end())
    mooseError(std::string("Invalid option \"") + name + "\" in MooseEnum.  Valid options are \"" + _raw_names << "\".");

  _current_name = name;
  _current_id = _name_to_id[name];
  return *this;
}

void
MooseEnum::fillNames(std::string names)
{
  _raw_names = names;

  std::istringstream iss(names);
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
