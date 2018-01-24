//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledMaterialDerivative.h"

template <>
InputParameters
validParams<CoupledMaterialDerivative>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription(
      "Kernel that implements (dFdv, test), where F is a property and v is a coupled variable");
  params.addRequiredCoupledVar("v", "Variable for the parital derivatives of the free energy");
  params.addParam<MaterialPropertyName>("f_name", "F", "The free energy material");
  params.addCoupledVar("args", "Vector of other nonlinear variables this object depends on");
  return params;
}

CoupledMaterialDerivative::CoupledMaterialDerivative(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _v_name(getVar("v", 0)->name()),
    _v_var(coupled("v")),
    _dFdv(getMaterialPropertyDerivative<Real>("f_name", _v_name)),
    _d2Fdvdu(getMaterialPropertyDerivative<Real>("f_name", _v_name, _var.name())),
    _d2Fdv2(getMaterialPropertyDerivative<Real>("f_name", _v_name, _v_name)),
    _nvar(_coupled_moose_vars.size()),
    _d2Fdvdarg(_nvar)
{
  // Get free energy derivatives for other coupled variables besides v
  for (unsigned int i = 0; i < _nvar; ++i)
  {
    MooseVariable * ivar = _coupled_moose_vars[i];
    _d2Fdvdarg[i] = &getMaterialPropertyDerivative<Real>("mob_name", _v_name, ivar->name());
  }
}

void
CoupledMaterialDerivative::initialSetup()
{
  validateNonlinearCoupling<Real>("f_name");
}

Real
CoupledMaterialDerivative::computeQpResidual()
{
  return _dFdv[_qp] * _test[_i][_qp];
}

Real
CoupledMaterialDerivative::computeQpJacobian()
{
  return _d2Fdvdu[_qp] * _test[_i][_qp] * _phi[_j][_qp];
}

Real
CoupledMaterialDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  // Jacobian w.r.t. v
  if (jvar == _v_var)
    return _d2Fdv2[_qp] * _test[_i][_qp] * _phi[_j][_qp];

  //  for all other vars get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  return (*_d2Fdvdarg[cvar])[_qp] * _test[_i][_qp] * _phi[_j][_qp];
}
