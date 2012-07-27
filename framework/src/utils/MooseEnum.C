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
#include "Parser.h"

#include <sstream>
#include <algorithm>
#include <iterator>


MooseEnum::MooseEnum(std::string names) :
    _raw_names(names),
    _current_id(-1)
{
  fillNames(names);
}

MooseEnum::MooseEnum(std::string names, std::string default_name) :
    _raw_names(names)
{
  fillNames(names);

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
  std::vector<std::string> elements;
  // split on whitespace
  Parser::tokenize(names, elements, 1, " \t\n\v\f\r");

  _names.resize(elements.size());
  int value=0;
  for (unsigned int i=0; i<elements.size(); ++i)
  {
    std::vector<std::string> name_value;
    // split on equals sign
    Parser::tokenize(elements[i], name_value, 1, "=");

    // See if there is a value supplied for this option
    mooseAssert(name_value.size() <= 2, "Invalid option supplied in MooseEnum");
    if (name_value.size() == 2)
    {
      std::istringstream iss(name_value[1]);
      iss >> value;
    }

    // preserve case for raw options, append to list
    if (i)
      _raw_names += ' ';
    _raw_names += name_value[0];

    // convert name to uppercase
    std::string upper(name_value[0]);
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    // populate internal datastructures
    _names[i] = upper;
    _name_to_id[upper] = value++;
  }
}
