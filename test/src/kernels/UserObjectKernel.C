//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UserObjectKernel.h"

registerMooseObject("MooseTestApp", UserObjectKernel);

InputParameters
UserObjectKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<UserObjectName>("user_object", "The name of user data object to use.");
  return params;
}

UserObjectKernel::UserObjectKernel(const InputParameters & params)
  : Kernel(params),
    _mutley(getUserObject<MTUserObject>(
        "user_object")) // get user-data object and cast it down so we can use it
{
}

UserObjectKernel::~UserObjectKernel() {}

Real
UserObjectKernel::computeQpResidual()
{
  Real val = _mutley.doSomething(); // let Mutley do something
  return -_test[_i][_qp] * val;
}
