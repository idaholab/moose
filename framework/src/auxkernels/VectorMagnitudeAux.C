//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "VectorMagnitudeAux.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", VectorMagnitudeAux);

InputParameters
VectorMagnitudeAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Creates a field representing the magnitude of three coupled "
                             "variables using an Euclidean norm.");
  params.addRequiredCoupledVar("x", "x-component of the vector");
  params.addCoupledVar("y", "y-component of the vector");
  params.addCoupledVar("z", "z-component of the vector");

  return params;
}

VectorMagnitudeAux::VectorMagnitudeAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _x(coupledValue("x")),
    _y(_mesh.dimension() >= 2 ? coupledValue("y") : _zero),
    _z(_mesh.dimension() >= 3 ? coupledValue("z") : _zero)
{
}

Real
VectorMagnitudeAux::computeValue()
{
  return std::sqrt((_x[_qp] * _x[_qp]) + (_y[_qp] * _y[_qp]) + (_z[_qp] * _z[_qp]));
}
