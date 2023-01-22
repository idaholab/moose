//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ACInterface.h"

registerMooseObject("PhaseFieldApp", ACInterface);

InputParameters
ACInterface::validParams()
{
  InputParameters params = JvarMapKernelInterface<Kernel>::validParams();
  params.addClassDescription("Gradient energy Allen-Cahn Kernel");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("kappa_name", "kappa_op", "The kappa used with the kernel");
  params.addParam<bool>("variable_L",
                        true,
                        "The mobility is a function of any MOOSE variable (if "
                        "this is set to false L must be constant over the "
                        "entire domain!)");
  return params;
}

ACInterface::ACInterface(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _L(getMaterialProperty<Real>("mob_name")),
    _kappa(getMaterialProperty<Real>("kappa_name")),
    _variable_L(getParam<bool>("variable_L")),
    _dLdop(getMaterialPropertyDerivative<Real>("mob_name", _var.name())),
    _d2Ldop2(getMaterialPropertyDerivative<Real>("mob_name", _var.name(), _var.name())),
    _dkappadop(getMaterialPropertyDerivative<Real>("kappa_name", _var.name())),
    _dLdarg(_n_args),
    _d2Ldargdop(_n_args),
    _d2Ldarg2(_n_args),
    _dkappadarg(_n_args),
    _gradarg(_n_args)
{
  // Get mobility and kappa derivatives and coupled variable gradients
  for (unsigned int i = 0; i < _n_args; ++i)
  {
    MooseVariable * ivar = _coupled_standard_moose_vars[i];
    const VariableName iname = ivar->name();
    if (iname == _var.name())
    {
      if (isCoupled("args"))
        paramError("args",
                   "The kernel variable should not be specified in the coupled `args` parameter.");
      else
        paramError("coupled_variables",
                   "The kernel variable should not be specified in the coupled `coupled_variables` "
                   "parameter.");
    }

    _dLdarg[i] = &getMaterialPropertyDerivative<Real>("mob_name", i);
    _dkappadarg[i] = &getMaterialPropertyDerivative<Real>("kappa_name", i);
    _d2Ldargdop[i] = &getMaterialPropertyDerivative<Real>("mob_name", iname, _var.name());

    _gradarg[i] = &(ivar->gradSln());

    _d2Ldarg2[i].resize(_n_args);
    for (unsigned int j = 0; j < _n_args; ++j)
      _d2Ldarg2[i][j] = &getMaterialPropertyDerivative<Real>("mob_name", i, j);
  }
}

void
ACInterface::initialSetup()
{
  validateCoupling<Real>("mob_name");
  validateCoupling<Real>("kappa_name");
}

RealGradient
ACInterface::gradL()
{
  RealGradient g = _grad_u[_qp] * _dLdop[_qp];
  for (unsigned int i = 0; i < _n_args; ++i)
    g += (*_gradarg[i])[_qp] * (*_dLdarg[i])[_qp];
  return g;
}

RealGradient
ACInterface::nablaLPsi()
{
  // sum is the product rule gradient \f$ \nabla (L\psi) \f$
  RealGradient sum = _L[_qp] * _grad_test[_i][_qp];

  if (_variable_L)
    sum += gradL() * _test[_i][_qp];

  return sum;
}

RealGradient
ACInterface::kappaNablaLPsi()
{
  return _kappa[_qp] * nablaLPsi();
}

Real
ACInterface::computeQpResidual()
{
  return _grad_u[_qp] * kappaNablaLPsi();
}

Real
ACInterface::computeQpJacobian()
{
  // dsum is the derivative \f$ \frac\partial{\partial \eta} \left( \nabla (L\psi) \right) \f$
  RealGradient dsum =
      (_dkappadop[_qp] * _L[_qp] + _kappa[_qp] * _dLdop[_qp]) * _phi[_j][_qp] * _grad_test[_i][_qp];

  // compute the derivative of the gradient of the mobility
  if (_variable_L)
  {
    RealGradient dgradL =
        _grad_phi[_j][_qp] * _dLdop[_qp] + _grad_u[_qp] * _phi[_j][_qp] * _d2Ldop2[_qp];

    for (unsigned int i = 0; i < _n_args; ++i)
      dgradL += (*_gradarg[i])[_qp] * _phi[_j][_qp] * (*_d2Ldargdop[i])[_qp];

    dsum += (_kappa[_qp] * dgradL + _dkappadop[_qp] * _phi[_j][_qp] * gradL()) * _test[_i][_qp];
  }

  return _grad_phi[_j][_qp] * kappaNablaLPsi() + _grad_u[_qp] * dsum;
}

Real
ACInterface::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  // dsum is the derivative \f$ \frac\partial{\partial \eta} \left( \nabla (L\psi) \right) \f$
  RealGradient dsum = ((*_dkappadarg[cvar])[_qp] * _L[_qp] + _kappa[_qp] * (*_dLdarg[cvar])[_qp]) *
                      _phi[_j][_qp] * _grad_test[_i][_qp];

  // compute the derivative of the gradient of the mobility
  if (_variable_L)
  {
    RealGradient dgradL = _grad_phi[_j][_qp] * (*_dLdarg[cvar])[_qp] +
                          _grad_u[_qp] * _phi[_j][_qp] * (*_d2Ldargdop[cvar])[_qp];

    for (unsigned int i = 0; i < _n_args; ++i)
      dgradL += (*_gradarg[i])[_qp] * _phi[_j][_qp] * (*_d2Ldarg2[cvar][i])[_qp];

    dsum += (_kappa[_qp] * dgradL + _dkappadop[_qp] * _phi[_j][_qp] * gradL()) * _test[_i][_qp];
  }

  return _grad_u[_qp] * dsum;
}
