//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledAllenCahn.h"

registerMooseObject("PhaseFieldApp", CoupledAllenCahn);

InputParameters
CoupledAllenCahn::validParams()
{
  InputParameters params = ACBulk<Real>::validParams();
  params.addClassDescription(
      "Coupled Allen-Cahn Kernel that uses a DerivativeMaterial Free Energy");
  params.addRequiredCoupledVar("v", "Coupled variable");
  params.addRequiredParam<MaterialPropertyName>(
      "f_name", "Base name of the free energy function F defined in a DerivativeParsedMaterial");
  return params;
}

CoupledAllenCahn::CoupledAllenCahn(const InputParameters & parameters)
  : ACBulk<Real>(parameters),
    _v_name(coupledName("v", 0)),
    _dFdV(getMaterialPropertyDerivative<Real>("f_name", _v_name)),
    _d2FdVdEta(getMaterialPropertyDerivative<Real>("f_name", _v_name, _var.name())),
    _d2FdVdarg(_n_args)
{
  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _n_args; ++i)
    _d2FdVdarg[i] = &getMaterialPropertyDerivative<Real>("f_name", _v_name, i);
}

void
CoupledAllenCahn::initialSetup()
{
  ACBulk<Real>::initialSetup();
  validateNonlinearCoupling<Real>("f_name");
}

Real
CoupledAllenCahn::computeDFDOP(PFFunctionType type)
{
  switch (type)
  {
    case Residual:
      return _dFdV[_qp];

    case Jacobian:
      return _d2FdVdEta[_qp] * _phi[_j][_qp];
  }

  mooseError("Internal error");
}

Real
CoupledAllenCahn::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  return ACBulk<Real>::computeQpOffDiagJacobian(jvar) +
         _L[_qp] * (*_d2FdVdarg[cvar])[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}
