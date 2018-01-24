//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SusceptibilityTimeDerivative.h"

template <>
InputParameters
validParams<SusceptibilityTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
  params.addClassDescription(
      "A modified time derivative Kernel that multiplies the time derivative "
      "of a variable by a generalized susceptibility");
  params.addRequiredParam<MaterialPropertyName>(
      "f_name", "Susceptibility function F defined in a FunctionMaterial");
  params.addCoupledVar("args", "Vector of arguments of the susceptibility");
  return params;
}

SusceptibilityTimeDerivative::SusceptibilityTimeDerivative(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<TimeDerivative>>(parameters),
    _Chi(getMaterialProperty<Real>("f_name")),
    _dChidu(getMaterialPropertyDerivative<Real>("f_name", _var.name())),
    _dChidarg(_coupled_moose_vars.size())
{
  // fetch derivatives
  for (unsigned int i = 0; i < _dChidarg.size(); ++i)
    _dChidarg[i] = &getMaterialPropertyDerivative<Real>("f_name", _coupled_moose_vars[i]->name());
}

void
SusceptibilityTimeDerivative::initialSetup()
{
  validateNonlinearCoupling<Real>("f_name");
}

Real
SusceptibilityTimeDerivative::computeQpResidual()
{
  return TimeDerivative::computeQpResidual() * _Chi[_qp];
}

Real
SusceptibilityTimeDerivative::computeQpJacobian()
{
  return TimeDerivative::computeQpJacobian() * _Chi[_qp] +
         TimeDerivative::computeQpResidual() * _dChidu[_qp] * _phi[_j][_qp];
}

Real
SusceptibilityTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  return TimeDerivative::computeQpResidual() * (*_dChidarg[cvar])[_qp] * _phi[_j][_qp];
}
