/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CoupledAllenCahn.h"

template <>
InputParameters
validParams<CoupledAllenCahn>()
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
    _v_name(getVar("v", 0)->name()),
    _nvar(_coupled_moose_vars.size()),
    _dFdV(getMaterialPropertyDerivative<Real>("f_name", _v_name)),
    _d2FdVdEta(getMaterialPropertyDerivative<Real>("f_name", _v_name, _var.name())),
    _d2FdVdarg(_nvar)
{
  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _nvar; ++i)
    _d2FdVdarg[i] =
        &getMaterialPropertyDerivative<Real>("f_name", _v_name, _coupled_moose_vars[i]->name());
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
