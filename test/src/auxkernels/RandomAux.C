//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RandomAux.h"
#include "RandomElementalUserObject.h"

registerMooseObject("MooseTestApp", RandomAux);

InputParameters
RandomAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addParam<UserObjectName>("random_user_object",
                                  "The RandomElementalUserObject to optionally use");
  params.addParam<bool>(
      "generate_integers", false, "Tells this object to use ints instead of Reals");

  return params;
}

RandomAux::RandomAux(const InputParameters & params)
  : AuxKernel(params),
    _random_uo(params.isParamValid("random_user_object")
                   ? &getUserObject<RandomElementalUserObject>("random_user_object")
                   : NULL),
    _generate_ints(getParam<bool>("generate_integers"))
{
  /**
   * This call turns on Random Number generation for this object, it can be called either in
   * the constructor or in initialSetup().
   */
  setRandomResetFrequency(EXEC_LINEAR);

  if (_random_uo)
  {
    if (isNodal())
      mooseError("Can't use an ElementUserObject with a nodal RandomAux");
    else if (_generate_ints)
      mooseError("Can't get ints out of the RandomElementalUserObject");
  }
}

RandomAux::~RandomAux() {}

Real
RandomAux::computeValue()
{
  if (_random_uo)
    // Use the coupled UO to return a value
    return _random_uo->getElementalValue(_current_elem->id());
  else if (_generate_ints)
    // Use the built-in long random number generator directly
    return getRandomLong();
  else
    // Use the built-in random number generator directly
    return getRandomReal();
}
