//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolyDiffusion.h"

registerMooseObject("MooseTestApp", PolyDiffusion);

InputParameters
PolyDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  return params;
}

PolyDiffusion::PolyDiffusion(const InputParameters & parameters) : Kernel(parameters) {}

Real
PolyDiffusion::computeQpResidual()
{
  Real a = libMesh::pi;
  Real b = 3;
  Real e = 4;
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  Real z = _q_point[_qp](2);
  Real t = _t;
  Real u = a * x * x * x * y * t + b * y * y * z + e * x * y * z * z * z * z;
  return _grad_test[_i][_qp] * (u * u - 2 * u + 2) * _grad_u[_qp];
  // We multiplied by our k(u).
}

Real
PolyDiffusion::computeQpJacobian()
{
  Real a = libMesh::pi;
  Real b = 3;
  Real e = 4;
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  Real z = _q_point[_qp](2);
  Real t = _t;
  Real u = a * x * x * x * y * t + b * y * y * z + e * x * y * z * z * z;
  return _grad_test[_i][_qp] * (u * u - 2 * u + 2) * _grad_phi[_j][_qp];
  // We multiplied by our k(u)
}
