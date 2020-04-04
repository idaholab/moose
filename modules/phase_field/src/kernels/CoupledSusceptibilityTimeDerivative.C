//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledSusceptibilityTimeDerivative.h"

registerMooseObject("PhaseFieldApp", CoupledSusceptibilityTimeDerivative);

InputParameters
CoupledSusceptibilityTimeDerivative::validParams()
{
  InputParameters params = JvarMapKernelInterface<CoupledTimeDerivative>::validParams();
  params.addClassDescription("A modified coupled time derivative Kernel that multiplies the time "
                             "derivative of a coupled variable by a generalized susceptibility");
  params.addRequiredParam<MaterialPropertyName>(
      "f_name", "Susceptibility function F defined in a FunctionMaterial");
  return params;
}

CoupledSusceptibilityTimeDerivative::CoupledSusceptibilityTimeDerivative(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<CoupledTimeDerivative>>(parameters),
    _F(getMaterialProperty<Real>("f_name")),
    _dFdu(getMaterialPropertyDerivative<Real>("f_name", _var.name())),
    _dFdarg(_n_args)
{
  // fetch derivatives
  for (unsigned int i = 0; i < _n_args; ++i)
    _dFdarg[i] = &getMaterialPropertyDerivative<Real>("f_name", i);
}

void
CoupledSusceptibilityTimeDerivative::initialSetup()
{
  validateNonlinearCoupling<Real>("f_name");
}

Real
CoupledSusceptibilityTimeDerivative::computeQpResidual()
{
  return CoupledTimeDerivative::computeQpResidual() * _F[_qp];
}

Real
CoupledSusceptibilityTimeDerivative::computeQpJacobian()
{
  return CoupledTimeDerivative::computeQpResidual() * _dFdu[_qp] * _phi[_j][_qp];
}

Real
CoupledSusceptibilityTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  if (jvar == _v_var)
    return CoupledTimeDerivative::computeQpOffDiagJacobian(jvar) * _F[_qp] +
           CoupledTimeDerivative::computeQpResidual() * _phi[_j][_qp] * (*_dFdarg[cvar])[_qp];

  return CoupledTimeDerivative::computeQpResidual() * _phi[_j][_qp] * (*_dFdarg[cvar])[_qp];
}
