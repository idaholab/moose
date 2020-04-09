//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialDerivativeTestKernel.h"

registerMooseObject("MooseApp", MaterialDerivativeTestKernel);

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
