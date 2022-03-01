//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatCoupledForce.h"

#include "MooseVariable.h"

registerMooseObject("MooseApp", MatCoupledForce);

InputParameters
MatCoupledForce::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addClassDescription(
      "Implements a forcing term RHS of the form PDE = RHS, where RHS = Sum_j c_j * m_j * v_j. "
      "c_j, m_j, and v_j are provided as real coefficients, material properties, and coupled "
      "variables, respectively.");
  params.addRequiredCoupledVar("v", "The coupled variables which provide the force");
  params.addParam<std::vector<Real>>(
      "coef", "Coefficents ($\\sigma$) multiplier for the coupled force term.");
  params.addParam<std::vector<MaterialPropertyName>>("material_properties",
                                                     "The coupled material properties.");
  return params;
}

MatCoupledForce::MatCoupledForce(const InputParameters & parameters)
  : Kernel(parameters),
    _n_coupled(coupledComponents("v")),
    _coupled_props(isParamValid("material_properties")),
    _v_var(coupledIndices("v")),
    _v(coupledValues("v")),
    _coef(isParamValid("coef") ? getParam<std::vector<Real>>("coef")
                               : std::vector<Real>(_n_coupled, 1))
{
  for (MooseIndex(_n_coupled) j = 0; j < _n_coupled; ++j)
  {
    _v_var_to_index[_v_var[j]] = j;

    if (_var.number() == _v_var[j])
      paramError("v",
                 "Coupled variable 'v' needs to be different from 'variable' with MatCoupledForce, "
                 "consider using Reaction or somethig similar");
  }

  if (isParamValid("coef") && _coef.size() != _n_coupled)
    paramError("coef", "Size of coef must be equal to size of v");

  if (_coupled_props)
  {
    _mat_props.resize(_n_coupled);
    std::vector<MaterialPropertyName> names =
        getParam<std::vector<MaterialPropertyName>>("material_properties");
    if (names.size() != _n_coupled)
      paramError("material_properties", "Size must be equal to number of coupled variables");
    for (unsigned int j = 0; j < _n_coupled; ++j)
      _mat_props[j] = &getMaterialPropertyByName<Real>(names[j]);
  }
}

Real
MatCoupledForce::computeQpResidual()
{
  Real r = 0;
  if (_coupled_props)
    for (unsigned int j = 0; j < _n_coupled; ++j)
      r += -_coef[j] * (*_mat_props[j])[_qp] * (*_v[j])[_qp];
  else
    for (unsigned int j = 0; j < _n_coupled; ++j)
      r += -_coef[j] * (*_v[j])[_qp];
  return r * _test[_i][_qp];
}

Real
MatCoupledForce::computeQpJacobian()
{
  return 0;
}

Real
MatCoupledForce::computeQpOffDiagJacobian(unsigned int jvar)
{
  auto p = _v_var_to_index.find(jvar);
  if (p == _v_var_to_index.end())
    return 0;

  if (_coupled_props)
    return -_coef[p->second] * (*_mat_props[p->second])[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  return -_coef[p->second] * _phi[_j][_qp] * _test[_i][_qp];
}
