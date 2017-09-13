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

#include "MaterialDerivativeTestKernel.h"

template <>
InputParameters
validParams<MaterialDerivativeTestKernel>()
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
