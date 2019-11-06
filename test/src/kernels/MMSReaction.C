//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MMSReaction.h"
#include "MooseMesh.h"

registerMooseObject("MooseTestApp", MMSReaction);

InputParameters
MMSReaction::validParams()
{
  InputParameters params = Kernel::validParams();
  return params;
}

MMSReaction::MMSReaction(const InputParameters & parameters)
  : Kernel(parameters), _mesh_dimension(_mesh.dimension())
{
}

Real
MMSReaction::computeQpResidual()
{
  Real a = libMesh::pi;
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  Real t = _t;
  if (_mesh_dimension == 3)
  {
    Real z = _q_point[_qp](2);
    Real u = std::sin(a * x * y * z * t);
    return _test[_i][_qp] * 2 * u * u;
  }
  else
  {
    Real z = 1.0;
    Real u = std::sin(a * x * y * z * t);
    return _test[_i][_qp] * 2 * u * u;
  }
}

Real
MMSReaction::computeQpJacobian()
{
  return 0; // We have no grad u.
}
