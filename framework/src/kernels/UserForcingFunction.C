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

#include "UserForcingFunction.h"
#include "Function.h"

template <>
InputParameters
validParams<UserForcingFunction>()
{
  return validParams<BodyForce>();
}

UserForcingFunction::UserForcingFunction(const InputParameters & parameters) : BodyForce(parameters)
{
  mooseDeprecated("UserForcingFunction has been replaced by BodyForce.");
}

Real
UserForcingFunction::f()
{
  mooseDeprecated("This method is a legacy method from UserForcingFunction, please update your "
                  "code to use the BodyForce object and the _function member variable instead.");
  return _function.value(_t, _q_point[_qp]);
}
