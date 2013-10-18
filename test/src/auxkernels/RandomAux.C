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

#include "RandomAux.h"
#include "RandomElementalUserObject.h"

template<>
InputParameters validParams<RandomAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addParam<UserObjectName>("random_user_object", "The RandomElementalUserObject to optionally use");
  params.addParam<bool>("generate_integers", false, "Tells this object to use ints instead of Reals");

  return params;
}

RandomAux::RandomAux(const std::string & name, InputParameters params) :
    AuxKernel(name, params),
    _random_uo(params.isParamValid("random_user_object") ? &getUserObject<RandomElementalUserObject>("random_user_object") : NULL),
    _generate_ints(getParam<bool>("generate_integers"))
{
  /**
   * This call turns on Random Number generation for this object, it can be called either in
   * the constructor or in initialSetup().
   */
  setRandomResetFrequency(EXEC_RESIDUAL);

  if (_random_uo)
  {
    if (isNodal())
      mooseError("Can't use an ElementUserObject with a nodal RandomAux");
    else if (_generate_ints)
      mooseError("Can't get ints out of the RandomElementalUserObject");
  }
}

RandomAux::~RandomAux()
{
}

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
