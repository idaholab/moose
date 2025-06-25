//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUNeumannBC.h"

registerMooseObject("MooseApp", KokkosNeumannBC);

InputParameters
KokkosNeumannBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
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

KokkosNeumannBC::KokkosNeumannBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _value(getParam<Real>("value"))
{
}
