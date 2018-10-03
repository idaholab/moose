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

template <>
InputParameters
validParams<NaNInterfaceTestKernel>()
{
  InputParameters params = validParams<Kernel>();

  params.addRequiredParam<UserObjectName>("nan_interface_test_fp",
                                          "NaNInterfaceTestFluidProperties user object name");

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
  return _nan_interface_test_fp.p_from_v_e(0, 0);
}
