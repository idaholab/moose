/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CoupledCoefReaction.h"

template<>
InputParameters validParams<CoupledCoefReaction>()
{
  InputParameters params = validParams<CoupledReaction>();
  params.addClassDescription("Kernel to add L*v, where L=mobility, v=coupled variable");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addCoupledVar("args", "Vector of nonlinear variable arguments this object depends on");
  return params;
}

CoupledCoefReaction::CoupledCoefReaction(const InputParameters & parameters) :
  DerivativeMaterialInterface<JvarMapInterface<CoupledReaction> >(parameters),
    _v_name(getVar("v", 0)->name()),
    _v(coupledValue("v")),
    _v_var(coupled("v")),
    _L(getMaterialProperty<Real>("mob_name")),
    _eta_name(_var.name()),
    _dLdop(getMaterialPropertyDerivative<Real>("mob_name", _eta_name)),
    _dLdv(getMaterialPropertyDerivative<Real>("mob_name", _v_name)),
    _nvar(_coupled_moose_vars.size()),
    _dLdarg(_nvar)
{
  // Get mobility derivatives
  for (unsigned int i = 0; i < _nvar; ++i)
  {
    MooseVariable *ivar = _coupled_moose_vars[i];
    _dLdarg[i] = &getMaterialPropertyDerivative<Real>("mob_name", ivar->name());
  }
}

void
CoupledCoefReaction::initialSetup()
{
  validateNonlinearCoupling<Real>("mob_name");
}

Real
CoupledCoefReaction::computeQpResidual()
{
  return _L[_qp] * CoupledReaction::computeQpResidual();
}

// Following is non-zero only if order parameter depends on eta
Real
CoupledCoefReaction::computeQpJacobian()
{
  return _dLdop[_qp] * -_v[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}

Real
CoupledCoefReaction::computeQpOffDiagJacobian(unsigned int jvar)
{
  // first handle the case where jvar is the coupled variable v being added to residual
  // the first term in the sum just multiplies by L which is always needed
  // the second term accounts for cases where L depends on v
  if (jvar == _v_var)
    return (_L[_qp] + _dLdv[_qp] * _v[_qp]) * CoupledReaction::computeQpOffDiagJacobian(jvar);

  //  for all other vars get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  return (*_dLdarg[cvar])[_qp] * _v[_qp] * CoupledReaction::computeQpOffDiagJacobian(jvar);
}
