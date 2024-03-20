//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HDGDiffusionBC.h"

// MOOSE includes
#include "Function.h"
#include "MooseVariableFE.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/utility.h"

// C++ includes
#include <cmath>

registerMooseObject("MooseApp", HDGDiffusionBC);

InputParameters
HDGDiffusionBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredParam<Real>("alpha", "Alpha");
  params.addParam<MaterialPropertyName>(
      "diff", 1, "The diffusion (or thermal conductivity or viscosity) coefficient.");
  params.addRequiredParam<FunctionName>("exact_soln", "The exact solution.");
  return params;
}

HDGDiffusionBC::HDGDiffusionBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _alpha(getParam<Real>("alpha")),
    _diff(getADMaterialProperty<Real>("diff")),
    _func(getFunction("exact_soln"))
{
}

ADReal
HDGDiffusionBC::computeQpResidual()
{
  ADReal r = 0.0;

  const Real h_elem = _current_elem_volume / _current_side_volume;
  const auto side_value = _func.value(_t, _q_point[_qp]);

  r -= _diff[_qp] * _grad_u[_qp] * _normals[_qp] * _test[_i][_qp];
  r -= _alpha / h_elem * _diff[_qp] * (side_value - _u[_qp]) * _test[_i][_qp];
  r += (side_value - _u[_qp]) * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp];

  return r;
}
