//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledInterfacialSourceGradient.h"

registerMooseObject("MooseTestApp", ADCoupledInterfacialSourceGradient);

InputParameters
ADCoupledInterfacialSourceGradient::validParams()
{
  InputParameters params = ADInterfaceKernel::validParams();
  params.addParam<MaterialPropertyName>("D", "D", "The diffusion coefficient.");
  params.addParam<MaterialPropertyName>(
      "D_neighbor", "D_neighbor", "The neighboring diffusion coefficient.");
  params.addRequiredCoupledVar(
      "var_source", "A variable whose gradient jump serves as the source term at the interface");
  return params;
}

ADCoupledInterfacialSourceGradient::ADCoupledInterfacialSourceGradient(
    const InputParameters & parameters)
  : ADInterfaceKernel(parameters),
    _D(getMaterialProperty<Real>("D")),
    _D_neighbor(getNeighborMaterialProperty<Real>("D_neighbor")),
    _grad_var(adCoupledGradient("var_source")),
    _grad_var_neighbor(adCoupledNeighborGradient("var_source"))
{
}

ADReal
ADCoupledInterfacialSourceGradient::computeQpResidual(Moose::DGResidualType type)
{
  ADReal gradient_jump = (_grad_var[_qp] - _grad_var_neighbor[_qp]) * _normals[_qp];

  ADReal r = 0;

  switch (type)
  {
    case Moose::Element:
      r = -_test[_i][_qp] *
          (_D_neighbor[_qp] * _grad_neighbor_value[_qp] * _normals[_qp] + gradient_jump);
      break;

    case Moose::Neighbor:
      r = _test_neighbor[_i][_qp] * (_D[_qp] * _grad_u[_qp] * _normals[_qp] - gradient_jump);
      break;
  }

  return r;
}
