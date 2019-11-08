//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RandomElementalUserObject.h"

registerMooseObject("MooseTestApp", RandomElementalUserObject);

InputParameters
RandomElementalUserObject::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;
  return params;
}

RandomElementalUserObject::RandomElementalUserObject(const InputParameters & parameters)
  : ElementUserObject(parameters)
{
  /**
   * This call turns on Random Number generation for this object, it can be called either in
   * the constructor or in initialSetup().
   */
  setRandomResetFrequency(EXEC_LINEAR);
}

RandomElementalUserObject::~RandomElementalUserObject() {}

void
RandomElementalUserObject::initialize()
{
  _random_data.clear();
}

void
RandomElementalUserObject::execute()
{
  _random_data[_current_elem->id()] = getRandomLong();
}

void
RandomElementalUserObject::finalize()
{
  _communicator.set_union(_random_data);
}

void
RandomElementalUserObject::threadJoin(const UserObject & y)
{
  const RandomElementalUserObject & uo = static_cast<const RandomElementalUserObject &>(y);

  _random_data.insert(uo._random_data.begin(), uo._random_data.end());
}

unsigned long
RandomElementalUserObject::getElementalValue(unsigned int element_id) const
{
  if (_random_data.find(element_id) == _random_data.end())
    return 0;
  else
    return _random_data.at(element_id);
}
