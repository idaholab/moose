//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DGMDDBC.h"

// MOOSE includes
#include "Function.h"
#include "MooseVariableFE.h"

#include "libmesh/utility.h"

// C++ includes
#include <cmath>

registerMooseObject("MooseTestApp", DGMDDBC);

InputParameters
DGMDDBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addParam<Real>("value", 0.0, "The value the variable should have on the boundary");
  params.addRequiredParam<FunctionName>("function", "The forcing function.");
  params.addRequiredParam<Real>("epsilon", "Epsilon");
  params.addRequiredParam<Real>("sigma", "Sigma");
  params.addRequiredParam<MaterialPropertyName>("prop_name", "diff1");

  return params;
}

DGMDDBC::DGMDDBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _func(getFunction("function")),
    _diff(getMaterialProperty<Real>("prop_name")),
    _epsilon(getParam<Real>("epsilon")),
    _sigma(getParam<Real>("sigma"))
{
}

Real
DGMDDBC::computeQpResidual()
{
  const int elem_b_order = std::max(libMesh::Order(1), _var.order());
  const Real h_elem =
      _current_elem_volume / _current_side_volume * 1.0 / Utility::pow<2>(elem_b_order);

  const Real fn = _func.value(_t, _q_point[_qp]);
  Real r = 0;
  r -= (_diff[_qp] * _grad_u[_qp] * _normals[_qp] * _test[_i][_qp]);
  r += _epsilon * (_u[_qp] - fn) * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp];
  r += _sigma / h_elem * (_u[_qp] - fn) * _test[_i][_qp];

  return r;
}

Real
DGMDDBC::computeQpJacobian()
{
  const int elem_b_order = std::max(libMesh::Order(1), _var.order());
  const Real h_elem =
      _current_elem_volume / _current_side_volume * 1.0 / Utility::pow<2>(elem_b_order);

  Real r = 0;
  r -= _diff[_qp] * _grad_test[_j][_qp] * _normals[_qp] * _test[_i][_qp];
  r += _epsilon * _test[_j][_qp] * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp];
  r += _sigma / h_elem * _test[_j][_qp] * _test[_i][_qp];
  return r;
}
