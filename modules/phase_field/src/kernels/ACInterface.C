/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACInterface.h"

template <>
InputParameters
validParams<ACInterface>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Gradient energy Allen-Cahn Kernel");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("kappa_name", "kappa_op", "The kappa used with the kernel");
  params.addCoupledVar("args", "Vector of nonlinear variable arguments this object depends on");
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
    _nvar(_coupled_moose_vars.size()),
    _dLdarg(_nvar),
    _d2Ldargdop(_nvar),
    _d2Ldarg2(_nvar),
    _dkappadarg(_nvar),
    _gradarg(_nvar)
{
  // Get mobility and kappa derivatives and coupled variable gradients
  for (unsigned int i = 0; i < _nvar; ++i)
  {
    MooseVariable * ivar = _coupled_moose_vars[i];
    const VariableName iname = ivar->name();
    if (iname == _var.name())
      mooseError("The kernel variable should not be specified in the coupled `args` parameter.");

    _dLdarg[i] = &getMaterialPropertyDerivative<Real>("mob_name", iname);
    _dkappadarg[i] = &getMaterialPropertyDerivative<Real>("kappa_name", iname);

    _d2Ldargdop[i] = &getMaterialPropertyDerivative<Real>("mob_name", iname, _var.name());

    _gradarg[i] = &(ivar->gradSln());

    _d2Ldarg2[i].resize(_nvar);
    for (unsigned int j = 0; j < _nvar; ++j)
      _d2Ldarg2[i][j] =
          &getMaterialPropertyDerivative<Real>("mob_name", iname, _coupled_moose_vars[j]->name());
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
  for (unsigned int i = 0; i < _nvar; ++i)
    g += (*_gradarg[i])[_qp] * (*_dLdarg[i])[_qp];
  return g;
}

RealGradient
ACInterface::kappaNablaLPsi()
{
  // sum is the product rule gradient \f$ \nabla (L\psi) \f$
  RealGradient sum = _L[_qp] * _grad_test[_i][_qp];

  if (_variable_L)
    sum += gradL() * _test[_i][_qp];

  return _kappa[_qp] * sum;
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

    for (unsigned int i = 0; i < _nvar; ++i)
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

    for (unsigned int i = 0; i < _nvar; ++i)
      dgradL += (*_gradarg[i])[_qp] * _phi[_j][_qp] * (*_d2Ldarg2[cvar][i])[_qp];

    dsum += (_kappa[_qp] * dgradL + _dkappadop[_qp] * _phi[_j][_qp] * gradL()) * _test[_i][_qp];
  }

  return _grad_u[_qp] * dsum;
}
