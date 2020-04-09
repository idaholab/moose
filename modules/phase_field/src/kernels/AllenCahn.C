//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AllenCahn.h"

registerMooseObject("PhaseFieldApp", AllenCahn);

InputParameters
AllenCahn::validParams()
{
  InputParameters params = ACBulk<Real>::validParams();
  params.addClassDescription("Allen-Cahn Kernel that uses a DerivativeMaterial Free Energy");
  params.addRequiredParam<MaterialPropertyName>(
      "f_name", "Base name of the free energy function F defined in a DerivativeParsedMaterial");
  return params;
}

AllenCahn::AllenCahn(const InputParameters & parameters)
  : ACBulk<Real>(parameters),
    _dFdEta(getMaterialPropertyDerivative<Real>("f_name", _var.name())),
    _d2FdEta2(getMaterialPropertyDerivative<Real>("f_name", _var.name(), _var.name())),
    _d2FdEtadarg(_n_args)
{
  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _n_args; ++i)
    _d2FdEtadarg[i] = &getMaterialPropertyDerivative<Real>("f_name", _var.name(), i);
}

void
AllenCahn::initialSetup()
{
  ACBulk<Real>::initialSetup();
  validateNonlinearCoupling<Real>("f_name");
  validateDerivativeMaterialPropertyBase<Real>("f_name");
}

Real
AllenCahn::computeDFDOP(PFFunctionType type)
{
  switch (type)
  {
    case Residual:
      return _dFdEta[_qp];

    case Jacobian:
      return _d2FdEta2[_qp] * _phi[_j][_qp];
  }

  mooseError("Internal error");
}

Real
AllenCahn::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  return ACBulk<Real>::computeQpOffDiagJacobian(jvar) +
         _L[_qp] * (*_d2FdEtadarg[cvar])[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}
