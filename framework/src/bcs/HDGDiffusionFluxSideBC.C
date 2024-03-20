//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HDGDiffusionFluxSideBC.h"

// MOOSE includes
#include "Function.h"
#include "MooseVariableFE.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/utility.h"

// C++ includes
#include <cmath>

registerMooseObject("MooseApp", HDGDiffusionFluxSideBC);

InputParameters
HDGDiffusionFluxSideBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredParam<Real>("alpha", "Alpha");
  params.addParam<MaterialPropertyName>(
      "diff", 1, "The diffusion (or thermal conductivity or viscosity) coefficient.");
  params.addRequiredCoupledVar("interior_variable", "interior variable");
  params.addParam<FunctionName>("flux", 0, "The flux in the normal direction");
  return params;
}

HDGDiffusionFluxSideBC::HDGDiffusionFluxSideBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _alpha(getParam<Real>("alpha")),
    _diff(getADMaterialProperty<Real>("diff")),
    _interior_value(adCoupledValue("interior_variable")),
    _interior_gradient(adCoupledGradient("interior_variable")),
    _flux(getFunction("flux"))
{
}

ADReal
HDGDiffusionFluxSideBC::computeQpResidual()
{
  ADReal r = 0.0;

  const Real h_elem = _current_elem_volume / _current_side_volume;

  r += _diff[_qp] * _interior_gradient[_qp] * _normals[_qp] * _test[_i][_qp];
  r += _alpha / h_elem * _diff[_qp] * (_u[_qp] - _interior_value[_qp]) * _test[_i][_qp];
  r += _flux.value(_t, _q_point[_qp]) * _test[_i][_qp];

  return r;
}
