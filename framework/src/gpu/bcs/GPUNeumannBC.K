//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUNeumannBC.h"

registerMooseObject("MooseApp", GPUNeumannBC);

InputParameters
GPUNeumannBC::validParams()
{
  InputParameters params = GPUIntegratedBC::validParams();
  params.addParam<Real>("value",
                        0.0,
                        "For a Laplacian problem, the value of the gradient dotted with the "
                        "normals on the boundary.");
  params.declareControllable("value");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{\\partial u}{\\partial n}=h$, "
                             "where $h$ is a constant, controllable value.");
  return params;
}

GPUNeumannBC::GPUNeumannBC(const InputParameters & parameters)
  : GPUIntegratedBC(parameters), _value(getParam<Real>("value"))
{
}
