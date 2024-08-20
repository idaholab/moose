//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatReaction.h"

registerMooseObject("MooseApp", MatReaction);

InputParameters
MatReaction::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addCoupledVar("v",
                       "Set this to make v a coupled variable, otherwise it will use the "
                       "kernel's nonlinear variable for v");
  params.addClassDescription("Kernel to add -L*v, where L=reaction rate, v=variable");
  params.addRequiredParam<MaterialPropertyName>("mob_name",
                                                "The reaction rate used with the kernel");
  params.deprecateParam("mob_name", "reaction_rate", "01/01/2025");
  params.addCoupledVar("args", "Vector of nonlinear variable arguments this object depends on");
  return params;
}

MatReaction::MatReaction(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _is_coupled(isCoupled("v")),
    _v_name(_is_coupled ? coupledName("v") : _var.name()),
    _v(_is_coupled ? coupledValue("v") : _u),
    _v_var(_is_coupled ? coupled("v") : _var.number()),
    _L(getMaterialProperty<Real>("reaction_rate")),
    _eta_name(_var.name()),
    _dLdop(getMaterialPropertyDerivative<Real>("reaction_rate", _eta_name)),
    _dLdv(getMaterialPropertyDerivative<Real>("reaction_rate", _v_name)),
    _dLdarg(_n_args)
{
  // Get reaction rate derivatives
  for (unsigned int i = 0; i < _n_args; ++i)
    _dLdarg[i] = &getMaterialPropertyDerivative<Real>("reaction_rate", i);
}

void
MatReaction::initialSetup()
{
  validateNonlinearCoupling<Real>("reaction_rate");
}

Real
MatReaction::computeQpResidual()
{
  return -_L[_qp] * _test[_i][_qp] * _v[_qp];
}

Real
MatReaction::computeQpJacobian()
{
  if (_is_coupled)
    return -_dLdop[_qp] * _v[_qp] * _phi[_j][_qp] * _test[_i][_qp];

  return -(_L[_qp] + _dLdop[_qp] * _v[_qp]) * _phi[_j][_qp] * _test[_i][_qp];
}

Real
MatReaction::computeQpOffDiagJacobian(unsigned int jvar)
{
  // first handle the case where jvar is a coupled variable v being added to residual
  // the first term in the sum just multiplies by L which is always needed
  // the second term accounts for cases where L depends on v
  if ((jvar == _v_var) && _is_coupled)
    return -(_L[_qp] + _dLdv[_qp] * _v[_qp]) * _test[_i][_qp] * _phi[_j][_qp];

  //  for all other vars get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  return -(*_dLdarg[cvar])[_qp] * _v[_qp] * _test[_i][_qp] * _phi[_j][_qp];
}
