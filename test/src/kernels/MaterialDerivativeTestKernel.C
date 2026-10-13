//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialDerivativeTestKernel.h"

/**
 * This file is used in the tests for the thermal hydraulics app
 * to try and prevent duplicate files we simlink to this in the module's test directory
 * the ifdef allows it to be registered with MooseTestApp when testing in moose/test
 * and to be registered with ThermalHydraulicsTestApp when building that module
 * see PR #33926 for a more detailed discussion
 */
#ifdef THERMAL_HYDRAULICS_ENABLED
registerMooseObject("ThermalHydraulicsTestApp", MaterialDerivativeTestKernel);
#else
registerMooseObject("MooseTestApp", MaterialDerivativeTestKernel);
#endif

InputParameters
MaterialDerivativeTestKernel::validParams()
{
  InputParameters params = MaterialDerivativeTestKernelBase<Real>::validParams();
  params.addClassDescription("Class used for testing derivatives of a scalar material property.");
  return params;
}

MaterialDerivativeTestKernel::MaterialDerivativeTestKernel(const InputParameters & parameters)
  : MaterialDerivativeTestKernelBase<Real>(parameters)
{
}

Real
MaterialDerivativeTestKernel::computeQpResidual()
{
  return _p[_qp] * _test[_i][_qp];
}

Real
MaterialDerivativeTestKernel::computeQpJacobian()
{
  return _p_diag_derivative[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}

Real
MaterialDerivativeTestKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable number corresponding to jvar
  const unsigned int cvar = mapJvarToCvar(jvar);
  return (*_p_off_diag_derivatives[cvar])[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}
