//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolyReaction.h"

registerMooseObject("MooseTestApp", PolyReaction);

InputParameters
PolyReaction::validParams()
{
  InputParameters params = Kernel::validParams();
  return params;
}

PolyReaction::PolyReaction(const InputParameters & parameters) : Kernel(parameters) {}

Real
PolyReaction::computeQpResidual()
{
  Real a = libMesh::pi;
  Real b = 3;
  Real e = 4;
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  Real z = _q_point[_qp](2);
  Real t = _t;
  Real u = a * x * x * x * y * t + b * y * y * z + e * x * y * z * z * z * z;
  return _test[_i][_qp] * 2 * u * u;
}

Real
PolyReaction::computeQpJacobian()
{
  return 0; // We have no grad u.
}
