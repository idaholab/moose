//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolyConstantAux.h"

registerMooseObject("MooseTestApp", PolyConstantAux);

InputParameters
PolyConstantAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  return params;
}

PolyConstantAux::PolyConstantAux(const InputParameters & parameters) : AuxKernel(parameters) {}

Real
PolyConstantAux::computeValue()
{
  Real a = libMesh::pi;
  Real b = 3;
  Real e = 4;
  Real x = (*_current_node)(0);
  Real y = (*_current_node)(1);
  Real z = (*_current_node)(2);
  Real t = _t;
  return a * x * x * x * y * t + b * y * y * z + e * x * y * z * z * z * z;
}
