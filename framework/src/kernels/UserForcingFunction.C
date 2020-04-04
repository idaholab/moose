//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UserForcingFunction.h"
#include "Function.h"

registerMooseObjectRenamed("MooseApp", UserForcingFunction, "04/01/2018 00:00", BodyForce);

InputParameters
UserForcingFunction::validParams()
{
  return BodyForce::validParams();
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
