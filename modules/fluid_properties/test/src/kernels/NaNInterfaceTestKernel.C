//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NaNInterfaceTestKernel.h"
#include "NaNInterfaceTestFluidProperties.h"

registerMooseObject("FluidPropertiesTestApp", NaNInterfaceTestKernel);

InputParameters
NaNInterfaceTestKernel::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addRequiredParam<UserObjectName>("nan_interface_test_fp",
                                          "NaNInterfaceTestFluidProperties user object name");
  params.addParam<bool>("test_vector_version", false, "Test getNaNVector? Else, test getNaN");

  params.addClassDescription("Kernel to test NaNInterface using NaNInterfaceTestFluidProperties");

  return params;
}

NaNInterfaceTestKernel::NaNInterfaceTestKernel(const InputParameters & parameters)
  : Kernel(parameters),
    _nan_interface_test_fp(getUserObject<NaNInterfaceTestFluidProperties>("nan_interface_test_fp"))
{
}

Real
NaNInterfaceTestKernel::computeQpResidual()
{
  if (getParam<bool>("test_vector_version"))
  {
    const std::vector<Real> nan_vector = _nan_interface_test_fp.returnNaNVector();
    return nan_vector[0];
  }
  else
    return _nan_interface_test_fp.p_from_v_e(0, 0);
}
