//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HDGDiffusionFluxBC.h"

// MOOSE includes
#include "Function.h"
#include "MooseVariableFE.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/utility.h"

// C++ includes
#include <cmath>

registerMooseObject("MooseApp", HDGDiffusionFluxBC);

InputParameters
HDGDiffusionFluxBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredParam<Real>("alpha", "Alpha");
  params.addParam<MaterialPropertyName>(
      "diff", 1, "The diffusion (or thermal conductivity or viscosity) coefficient.");
  params.addRequiredCoupledVar("side_variable", "side variable to use as Lagrange multiplier");
  return params;
}

HDGDiffusionFluxBC::HDGDiffusionFluxBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _alpha(getParam<Real>("alpha")),
    _diff(getADMaterialProperty<Real>("diff")),
    _side_u(adCoupledValue("side_variable"))
{
}

ADReal
HDGDiffusionFluxBC::computeQpResidual()
{
  ADReal r = 0.0;

  const Real h_elem = _current_elem_volume / _current_side_volume;

  r -= _diff[_qp] * _grad_u[_qp] * _normals[_qp] * _test[_i][_qp];
  r -= _alpha / h_elem * _diff[_qp] * (_side_u[_qp] - _u[_qp]) * _test[_i][_qp];
  r += (_side_u[_qp] - _u[_qp]) * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp];

  return r;
}
