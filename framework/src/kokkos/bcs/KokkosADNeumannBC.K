//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADNeumannBC.h"

registerKokkosADResidualObject("MooseApp", KokkosADNeumannBC);

InputParameters
KokkosADNeumannBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
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

KokkosADNeumannBC::KokkosADNeumannBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _value(getParam<Real>("value"))
{
}
