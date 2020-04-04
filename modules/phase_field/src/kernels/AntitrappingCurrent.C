//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AntitrappingCurrent.h"

registerMooseObject("PhaseFieldApp", AntitrappingCurrent);

InputParameters
AntitrappingCurrent::validParams()
{
  InputParameters params = CoupledSusceptibilityTimeDerivative::validParams();
  params.addClassDescription(
      "Kernel that provides antitrapping current at the interface for alloy solidification");
  return params;
}

AntitrappingCurrent::AntitrappingCurrent(const InputParameters & parameters)
  : CoupledSusceptibilityTimeDerivative(parameters), _grad_v(coupledGradient("v"))
{
}

Real
AntitrappingCurrent::computeQpResidual()
{
  const Real norm_sq = _grad_v[_qp].norm_sq();
  if (norm_sq < libMesh::TOLERANCE)
    return 0.0;

  return _F[_qp] * _v_dot[_qp] * _grad_v[_qp] * _grad_test[_i][_qp] / std::sqrt(norm_sq);
}

Real
AntitrappingCurrent::computeQpJacobian()
{
  const Real norm_sq = _grad_v[_qp].norm_sq();
  if (norm_sq < libMesh::TOLERANCE)
    return 0.0;

  return _dFdu[_qp] * _v_dot[_qp] * _grad_v[_qp] * _grad_test[_i][_qp] * _phi[_j][_qp] /
         std::sqrt(norm_sq);
}

Real
AntitrappingCurrent::computeQpOffDiagJacobian(unsigned int jvar)
{
  const Real norm_sq = _grad_v[_qp].norm_sq();
  if (norm_sq < libMesh::TOLERANCE)
    return 0.0;

  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  if (jvar == _v_var)
    return (_F[_qp] * _dv_dot[_qp] * _grad_v[_qp] * _grad_test[_i][_qp] * _phi[_j][_qp] +
            _F[_qp] * _v_dot[_qp] * _grad_test[_i][_qp] * _grad_phi[_j][_qp] -
            _F[_qp] * _v_dot[_qp] * _grad_v[_qp] * _grad_test[_i][_qp] * _grad_v[_qp] *
                _grad_phi[_j][_qp] / norm_sq +
            _v_dot[_qp] * _grad_v[_qp] * _grad_test[_i][_qp] * _phi[_j][_qp] *
                (*_dFdarg[cvar])[_qp]) /
           std::sqrt(norm_sq);

  return _v_dot[_qp] * _grad_v[_qp] * _grad_test[_i][_qp] * _phi[_j][_qp] * (*_dFdarg[cvar])[_qp] /
         std::sqrt(norm_sq);
}
