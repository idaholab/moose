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

#include "RandomElementalUserObject.h"

template <>
InputParameters
validParams<RandomElementalUserObject>()
{
  InputParameters params = validParams<ElementUserObject>();
  MooseUtils::setExecuteOnFlags(params, 1, EXEC_TIMESTEP_BEGIN);
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
