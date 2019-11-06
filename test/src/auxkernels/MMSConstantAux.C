//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MMSConstantAux.h"
#include "MooseMesh.h"

registerMooseObject("MooseTestApp", MMSConstantAux);

InputParameters
MMSConstantAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  return params;
}

MMSConstantAux::MMSConstantAux(const InputParameters & parameters)
  : AuxKernel(parameters), _mesh_dimension(_mesh.dimension())
{
}

Real
MMSConstantAux::computeValue()
{
  Real a = libMesh::pi;
  Real x = (*_current_node)(0);
  Real y = (*_current_node)(1);
  Real t = _t;

  if (_mesh_dimension == 3)
  {
    Real z = (*_current_node)(2);
    return std::sin(a * x * y * z * t);
  }
  else
  {
    Real z = 1.0;
    return std::sin(a * x * y * z * t);
  }
}
