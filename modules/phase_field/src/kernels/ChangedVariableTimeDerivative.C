//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChangedVariableTimeDerivative.h"

registerMooseObject("PhaseFieldApp", ChangedVariableTimeDerivative);

InputParameters
ChangedVariableTimeDerivative::validParams()
{
  InputParameters params = TimeDerivative::validParams();
  params.addClassDescription(
      "A modified time derivative Kernel that multiplies the time derivative by"
      "the derivative of the nonlinear preconditioning function");
  params.addRequiredParam<MaterialPropertyName>(
      "order_parameter",
      "Order parameter material defining the nonlinear preconditioning function");
  params.addCoupledVar("args", "Vector of additional arguments for order_parameter");
  return params;
}

ChangedVariableTimeDerivative::ChangedVariableTimeDerivative(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<TimeDerivative>>(parameters),
    _dopdu(getMaterialPropertyDerivative<Real>("order_parameter", _var.name())),
    _d2opdu2(getMaterialPropertyDerivative<Real>("order_parameter", _var.name(), _var.name())),
    _d2opdudarg(_n_args)
{
  // fetch derivatives
  for (unsigned int i = 0; i < _n_args; ++i)
    _d2opdudarg[i] = &getMaterialPropertyDerivative<Real>("order_parameter", _var.name(), i);
}

void
ChangedVariableTimeDerivative::initialSetup()
{
  validateNonlinearCoupling<Real>("order_parameter");
}

Real
ChangedVariableTimeDerivative::computeQpResidual()
{
  return TimeDerivative::computeQpResidual() * _dopdu[_qp];
}

Real
ChangedVariableTimeDerivative::computeQpJacobian()
{
  return TimeDerivative::computeQpJacobian() * _dopdu[_qp] +
         TimeDerivative::computeQpResidual() * _d2opdu2[_qp] * _phi[_j][_qp];
}

Real
ChangedVariableTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  return TimeDerivative::computeQpResidual() * (*_d2opdudarg[cvar])[_qp] * _phi[_j][_qp];
}
