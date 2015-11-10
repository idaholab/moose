/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACBulk.h"

template<>
InputParameters validParams<ACBulk>()
{
  InputParameters params = validParams<KernelValue>();
  params.addClassDescription("Allen-Cahn bulk energy kernel");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addCoupledVar("args", "Vector of nonlinear variable arguments this object depends on");
  return params;
}

ACBulk::ACBulk(const InputParameters & parameters) :
    DerivativeMaterialInterface<JvarMapInterface<KernelValue> >(parameters),
    _L(getMaterialProperty<Real>("mob_name")),
    _dLdop(getMaterialPropertyDerivative<Real>("mob_name", _var.name()))
{
  // Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _dLdarg.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
    _dLdarg[i] = &getMaterialPropertyDerivative<Real>("mob_name", _coupled_moose_vars[i]->name());
}

void
ACBulk::initialSetup()
{
  validateNonlinearCoupling<Real>("mob_name");
}

Real
ACBulk::precomputeQpResidual()
{
  // Get free energy derivative from function
  Real dFdop = computeDFDOP(Residual);

  // Set residual
  return  _L[_qp] * dFdop;
}

Real
ACBulk::precomputeQpJacobian()
{
  // Get free energy derivative and Jacobian
  Real dFdop = computeDFDOP(Residual);

  Real JdFdop = computeDFDOP(Jacobian);

  // Set Jacobian value using product rule
  return _L[_qp] * JdFdop + _dLdop[_qp] * _phi[_j][_qp] * dFdop;
}

Real
ACBulk::computeQpOffDiagJacobian(unsigned int jvar)
{
  // Get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  // Set off-diagonal Jacobian term from mobility derivatives
  return (*_dLdarg[cvar])[_qp] * _phi[_j][_qp] * computeDFDOP(Residual) * _test[_i][_qp];
}
