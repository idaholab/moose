//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorNeumannBC.h"

registerMooseObject("MooseApp", VectorNeumannBC);

InputParameters
VectorNeumannBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addParam<RealVectorValue>(
      "vector_value", RealVectorValue(), "vector this BC should act in");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{\\partial u}{\\partial n}=\\vec{V}\\cdot\\hat{n}$, "
                             "where $\\vec{V}$ is a user-defined, constant vector.");
  return params;
}

VectorNeumannBC::VectorNeumannBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _value(getParam<RealVectorValue>("vector_value"))
{
}

Real
VectorNeumannBC::computeQpResidual()
{
  return -_test[_i][_qp] * (_value * _normals[_qp]);
}
