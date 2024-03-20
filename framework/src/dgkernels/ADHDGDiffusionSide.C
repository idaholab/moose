//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHDGDiffusionSide.h"

// MOOSE includes
#include "MooseVariableFE.h"

#include "libmesh/utility.h"

registerMooseObject("MooseApp", ADHDGDiffusionSide);

InputParameters
ADHDGDiffusionSide::validParams()
{
  InputParameters params = ADDGKernel::validParams();
  // See header file for alpha
  params.addRequiredParam<Real>("alpha", "alpha");
  params.addParam<MaterialPropertyName>(
      "diff", 1., "The diffusion (or thermal conductivity or viscosity) coefficient.");
  params.addRequiredCoupledVar("interior_variable", "interior variable to find jumps in");
  return params;
}

ADHDGDiffusionSide::ADHDGDiffusionSide(const InputParameters & parameters)
  : ADDGKernel(parameters),
    _alpha(getParam<Real>("alpha")),
    _diff(getADMaterialProperty<Real>("diff")),
    _diff_neighbor(getNeighborADMaterialProperty<Real>("diff")),
    _interior_value(adCoupledValue("interior_variable")),
    _interior_neighbor_value(adCoupledNeighborValue("interior_variable")),
    _interior_gradient(adCoupledGradient("interior_variable")),
    _interior_neighbor_gradient(adCoupledNeighborGradient("interior_variable"))
{
}

ADReal
ADHDGDiffusionSide::computeQpResidual(Moose::DGResidualType type)
{
  ADReal r = 0.0;

  const Real h_elem = _current_elem_volume / _current_side_volume;

  switch (type)
  {
    case Moose::Element:
      r += _diff[_qp] * _interior_gradient[_qp] * _normals[_qp] * _test[_i][_qp];
      r += _alpha / h_elem * _diff[_qp] * (_u[_qp] - _interior_value[_qp]) * _test[_i][_qp];
      r -= _diff_neighbor[_qp] * _interior_neighbor_gradient[_qp] * _normals[_qp] * _test[_i][_qp];
      r += _alpha / h_elem * _diff_neighbor[_qp] * (_u[_qp] - _interior_neighbor_value[_qp]) *
           _test[_i][_qp];
      break;

    case Moose::Neighbor:
      // reverse sign for change in normals direction
      break;
  }

  return r;
}
