//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolyCoupledDirichletBC.h"

registerMooseObject("MooseTestApp", PolyCoupledDirichletBC);

InputParameters
PolyCoupledDirichletBC::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.set<bool>("_integrated") = false;
  params.addParam<Real>("value", 0.0, "Value multiplied by the coupled value on the boundary");

  return params;
}

PolyCoupledDirichletBC::PolyCoupledDirichletBC(const InputParameters & parameters)
  : NodalBC(parameters),
    // Grab the parameter for the multiplier.
    _value(getParam<Real>("value"))
{
}

Real
PolyCoupledDirichletBC::computeQpResidual()
{
  // We define all our variables here along with our function.
  Real a = libMesh::pi;
  Real b = 3;
  Real e = 4;
  Real x = (*_current_node)(0);
  Real y = (*_current_node)(1);
  Real z = (*_current_node)(2);
  Real t = _t;
  Real u = a * x * x * x * y * t + b * y * y * z + e * x * y * z * z * z * z;

  // Our function gets added here.
  return _u[_qp] - (u);
}
