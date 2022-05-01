//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DivergenceAux.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", DivergenceAux);

InputParameters
DivergenceAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription("Computes the divergence of a vector of field variables.");
  // Coupled variables
  params.addRequiredCoupledVar("u", "x-component of the vector");
  params.addCoupledVar("v", "y-component of the vector"); // only required in 2D and 3D
  params.addCoupledVar("w", "z-component of the vector"); // only required in 3D

  return params;
}

DivergenceAux::DivergenceAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _grad_u(coupledGradient("u")),
    _grad_v(_mesh.dimension() >= 2 ? coupledGradient("v") : _grad_zero),
    _grad_w(_mesh.dimension() == 3 ? coupledGradient("w") : _grad_zero)
{
}

Real
DivergenceAux::computeValue()
{
  // div U = du/dx + dv/dy + dw/dz
  return _grad_u[_qp](0) + _grad_v[_qp](1) + _grad_w[_qp](2);
}
