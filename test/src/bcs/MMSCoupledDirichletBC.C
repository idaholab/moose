//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MMSCoupledDirichletBC.h"
#include "MooseMesh.h"

registerMooseObject("MooseTestApp", MMSCoupledDirichletBC);

InputParameters
MMSCoupledDirichletBC::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.addParam<Real>("value", 0.0, "Value multiplied by the coupled value on the boundary");

  return params;
}

MMSCoupledDirichletBC::MMSCoupledDirichletBC(const InputParameters & parameters)
  : NodalBC(parameters),
    // Grab the parameter for the multiplier.
    _value(getParam<Real>("value")),
    _mesh_dimension(_mesh.dimension())
{
}

Real
MMSCoupledDirichletBC::computeQpResidual()
{
  // We define all our variables here along with our function.
  Real a = libMesh::pi;
  Real x = (*_current_node)(0);
  Real y = (*_current_node)(1);
  Real t = _t;
  if (_mesh_dimension == 3)
  {
    Real z = (*_current_node)(2);
    Real u = sin(a * x * y * z * t);
    // Our function gets added here.
    return _u[_qp] - u;
  }
  else
  {
    Real z = 1.0;
    Real u = sin(a * x * y * z * t);
    // Our function gets added here.
    return _u[_qp] - u;
  }
}
