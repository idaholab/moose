//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHDGDiffusion.h"

// MOOSE includes
#include "MooseVariableFE.h"

#include "libmesh/utility.h"

registerMooseObject("MooseApp", ADHDGDiffusion);

InputParameters
ADHDGDiffusion::validParams()
{
  InputParameters params = ADDGKernel::validParams();
  // See header file for alpha
  params.addRequiredParam<Real>("alpha", "alpha");
  params.addParam<MaterialPropertyName>(
      "diff", 1., "The diffusion (or thermal conductivity or viscosity) coefficient.");
  params.addRequiredCoupledVar("side_variable", "side variable to use as Lagrange multiplier");
  return params;
}

ADHDGDiffusion::ADHDGDiffusion(const InputParameters & parameters)
  : ADDGKernel(parameters),
    _alpha(getParam<Real>("alpha")),
    _diff(getADMaterialProperty<Real>("diff")),
    _diff_neighbor(getNeighborADMaterialProperty<Real>("diff")),
    _side_u(adCoupledValue("side_variable"))
{
}

ADReal
ADHDGDiffusion::computeQpResidual(Moose::DGResidualType type)
{
  ADReal r = 0.0;

  const Real h_elem = _current_elem_volume / _current_side_volume;

  switch (type)
  {
    case Moose::Element:
      r -= _diff[_qp] * _grad_u[_qp] * _normals[_qp] * _test[_i][_qp];
      r -= _alpha / h_elem * _diff[_qp] * (_side_u[_qp] - _u[_qp]) * _test[_i][_qp];
      r += (_side_u[_qp] - _u[_qp]) * _diff[_qp] * _grad_test[_i][_qp] * _normals[_qp];
      break;

    case Moose::Neighbor:
      // reverse sign for change in normals direction
      r += _diff_neighbor[_qp] * _grad_u_neighbor[_qp] * _normals[_qp] * _test_neighbor[_i][_qp];
      r -= _alpha / h_elem * _diff_neighbor[_qp] * (_side_u[_qp] - _u_neighbor[_qp]) *
           _test_neighbor[_i][_qp];
      r -= (_side_u[_qp] - _u_neighbor[_qp]) * _diff_neighbor[_qp] * _grad_test_neighbor[_i][_qp] *
           _normals[_qp];
      break;
  }

  return r;
}
