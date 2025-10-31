//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestADScalarKernel.h"
#include "TestADScalarKernelUserObject.h"

registerMooseObject("MooseTestApp", TestADScalarKernel);

InputParameters
TestADScalarKernel::validParams()
{
  InputParameters params = ADScalarKernel::validParams();

  params.addRequiredCoupledVar("v", "Coupled scalar variable");
  params.addRequiredParam<UserObjectName>("test_uo", "Test user object");

  return params;
}

TestADScalarKernel::TestADScalarKernel(const InputParameters & parameters)
  : ADScalarKernel(parameters),

    _v(adCoupledScalarValue("v")),
    _test_uo(getUserObject<TestADScalarKernelUserObject>("test_uo"))
{
}

ADReal
TestADScalarKernel::computeQpResidual()
{
  using std::pow;
  return 1.2 * pow(_u[0], 2) - 0.5 * pow(_v[0], 2) + _test_uo.getValue();
}
