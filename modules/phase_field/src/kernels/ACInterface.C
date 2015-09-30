/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACInterface.h"

template<>
InputParameters validParams<ACInterface>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Gradient energy Allen-Cahn Kernel");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("kappa_name", "kappa_op", "The kappa used with the kernel");
  params.addCoupledVar("args", "Vector of nonlinear variable arguments this object depends on");

  params.addParam<bool>("variable_L", true, "The mobility is a function of any non-linear variable");
  params.addParam<bool>("variable_kappa", false, "Kappa is a function of any non-linear variable");

  return params;
}

ACInterface::ACInterface(const InputParameters & parameters) :
    DerivativeMaterialInterface<JvarMapInterface<Kernel> >(parameters),
    _L(getMaterialProperty<Real>("mob_name")),
    _kappa(getMaterialProperty<Real>("kappa_name")),
    _variable_L(getParam<bool>("variable_L")),
    _variable_kappa(getParam<bool>("variable_kappa")),
    _dLdop(getMaterialPropertyDerivative<Real>("mob_name", _var.name())),
    _dkappadop(getMaterialPropertyDerivative<Real>("kappa_name", _var.name())),
    _nvar(_coupled_moose_vars.size()),
    _dLdarg(_nvar),
    _dkappadarg(_nvar),
    _gradarg(_nvar)
{
  // Get mobility and kappa derivatives and coupled variable gradients
  for (unsigned int i = 0; i < _nvar; ++i)
  {
    MooseVariable *cvar = _coupled_moose_vars[i];
    _dLdarg[i] = &getMaterialPropertyDerivative<Real>("mob_name", cvar->name());
    _dkappadarg[i] = &getMaterialPropertyDerivative<Real>("kappa_name", cvar->name());
    _gradarg[i] = &(cvar->gradSln());
  }
}

void
ACInterface::initialSetup()
{
  validateNonlinearCoupling<Real>("mob_name");
}

Real
ACInterface::computeQpResidual()
{
  // sum is the product rule gradient \f$ \nable (L\kappa\psi) \f$
  RealGradient sum = _kappa[_qp] * _L[_qp] * _grad_test[_i][_qp];

  // compute the gradient of the mobility
  if (_variable_L)
  {
    RealGradient gradL = _grad_u[_qp] * _dLdop[_qp];
    for (unsigned int i = 0; i < _nvar; ++i)
      gradL += (*_gradarg[i])[_qp] * (*_dLdarg[i])[_qp];
    sum += _kappa[_qp] * gradL * _test[_i][_qp];
  }

  // todo: kappa gradients!
  if (_variable_kappa)
  {
    RealGradient gradkappa = _grad_u[_qp] * _dkappadop[_qp];
    for (unsigned int i = 0; i < _nvar; ++i)
      gradkappa += (*_gradarg[i])[_qp] * (*_dkappadarg[i])[_qp];
    sum += gradkappa * _L[_qp] * _test[_i][_qp];

    // d/dop from the functional derivative
    //kappa_term = _dkappadop[_qp]/2.0 * (_grad_u[_qp] * _grad_u[_qp]) * _test[_i][_qp];
    // TODO: kappa could be a function of a coupled variable that a second ACInterface term is acting on. That derivative should appear here as well!
  }

  return _grad_u[_qp] * sum;
}

Real
ACInterface::computeQpJacobian()
{
  // Set Jacobian using product rule: TODO add the new terms
  return _kappa[_qp] * (_L[_qp] * _grad_phi[_j][_qp] + _dLdop[_qp] * _phi[_j][_qp] * _grad_u[_qp]) * _grad_test[_i][_qp];
}

Real
ACInterface::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  // Set off-diagonal jaocbian terms from mobility dependence
  return _kappa[_qp] * (*_dLdarg[cvar])[_qp] * _phi[_j][_qp] * _grad_u[_qp] * _grad_test[_i][_qp];
}
