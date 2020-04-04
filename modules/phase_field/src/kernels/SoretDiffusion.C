//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SoretDiffusion.h"

// MOOSE includes
#include "MooseVariable.h"

registerMooseObject("PhaseFieldApp", SoretDiffusion);

InputParameters
SoretDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Add Soret effect to Split formulation Cahn-Hilliard Kernel");
  params.addRequiredCoupledVar("T", "Temperature");
  params.addCoupledVar("c", "Concentration");
  params.addRequiredParam<MaterialPropertyName>("diff_name",
                                                "The diffusivity used with the kernel");
  params.addParam<MaterialPropertyName>(
      "Q_name", "Qheat", "The material name for the heat of transport");
  return params;
}

SoretDiffusion::SoretDiffusion(const InputParameters & parameters)
  : Kernel(parameters),
    _T_var(coupled("T")),
    _T(coupledValue("T")),
    _grad_T(coupledGradient("T")),
    _is_coupled(isCoupled("c")),
    _c_var(_is_coupled ? coupled("c") : _var.number()),
    _c(_is_coupled ? coupledValue("c") : _u),
    _D(getMaterialProperty<Real>("diff_name")),
    _Q(getMaterialProperty<Real>("Q_name")),
    _kB(8.617343e-5) // Boltzmann constant in eV/K
{
}

Real
SoretDiffusion::computeQpResidual()
{
  const Real T_term = _D[_qp] * _Q[_qp] * _c[_qp] / (_kB * _T[_qp] * _T[_qp]);
  return T_term * _grad_T[_qp] * _grad_test[_i][_qp];
}

Real
SoretDiffusion::computeQpJacobian()
{
  return (_is_coupled && _c_var != _var.number()) ? 0.0 : computeQpCJacobian();
}

Real
SoretDiffusion::computeQpOffDiagJacobian(unsigned int jvar)
{
  // c Off-Diagonal Jacobian
  if (_c_var == jvar)
    return computeQpCJacobian();

  // T Off-Diagonal Jacobian
  if (_T_var == jvar)
    return _D[_qp] * _Q[_qp] * _c[_qp] * _grad_test[_i][_qp] *
           (_grad_phi[_j][_qp] - 2.0 * _grad_T[_qp] * _phi[_j][_qp] / _T[_qp]) /
           (_kB * _T[_qp] * _T[_qp]);

  return 0.0;
}

Real
SoretDiffusion::computeQpCJacobian()
{
  // Calculate the Jacobian for the c variable
  return _D[_qp] * _Q[_qp] * _phi[_j][_qp] * _grad_T[_qp] / (_kB * _T[_qp] * _T[_qp]) *
         _grad_test[_i][_qp];
}
