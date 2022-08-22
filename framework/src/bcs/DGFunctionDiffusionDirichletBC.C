//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DGFunctionDiffusionDirichletBC.h"

// MOOSE includes
#include "Function.h"
#include "MooseVariableFE.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/utility.h"

// C++ includes
#include <cmath>

registerMooseObject("MooseApp", DGFunctionDiffusionDirichletBC);

InputParameters
DGFunctionDiffusionDirichletBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription(
      "Diffusion Dirichlet boundary condition for discontinuous Galerkin method.");
  params.addParam<Real>("value", 0.0, "The value the variable should have on the boundary");
  params.addRequiredParam<FunctionName>("function", "The forcing function.");
  params.addRequiredParam<Real>("epsilon", "Epsilon");
  params.addRequiredParam<Real>("sigma", "Sigma");
  params.addParam<MaterialPropertyName>(
      "diff", 1, "The diffusion (or thermal conductivity or viscosity) coefficient.");

  return params;
}

DGFunctionDiffusionDirichletBC::DGFunctionDiffusionDirichletBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _func(getFunction("function")),
    _epsilon(getParam<Real>("epsilon")),
    _sigma(getParam<Real>("sigma")),
    _diff(getMaterialProperty<Real>("diff"))
{
}

Real
DGFunctionDiffusionDirichletBC::computeQpResidual()
{
  const int elem_b_order = std::max(libMesh::Order(1), _var.order());
  const Real h_elem =
      _current_elem_volume / _current_side_volume * 1.0 / Utility::pow<2>(elem_b_order);

  const Real fn = _func.value(_t, _q_point[_qp]);
  Real r = 0.0;
  r -= (_diff[_qp] * _grad_u[_qp] * _normals[_qp] * _test[_i][_qp]);
  r += _epsilon * (_u[_qp] - fn) * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp];
  r += _sigma / h_elem * (_u[_qp] - fn) * _test[_i][_qp];

  return r;
}

Real
DGFunctionDiffusionDirichletBC::computeQpJacobian()
{
  const int elem_b_order = std::max(libMesh::Order(1), _var.order());
  const Real h_elem =
      _current_elem_volume / _current_side_volume * 1.0 / Utility::pow<2>(elem_b_order);

  Real r = 0.0;
  r -= (_diff[_qp] * _grad_phi[_j][_qp] * _normals[_qp] * _test[_i][_qp]);
  r += _epsilon * _phi[_j][_qp] * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp];
  r += _sigma / h_elem * _phi[_j][_qp] * _test[_i][_qp];

  return r;
}
