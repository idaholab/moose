//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayNeumannBC.h"

registerMooseObject("MooseApp", ArrayNeumannBC);

InputParameters
ArrayNeumannBC::validParams()
{
  InputParameters params = ArrayIntegratedBC::validParams();
  params.addParam<RealEigenVector>("value", "The value of the gradient on the boundary.");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{\\partial u}{\\partial n}=h$, "
                             "where $h$ is a constant, controllable value.");
  return params;
}

ArrayNeumannBC::ArrayNeumannBC(const InputParameters & parameters)
  : ArrayIntegratedBC(parameters),
    _value(isParamValid("value") ? getParam<RealEigenVector>("value")
                                 : RealEigenVector::Zero(_count))
{
}

void
ArrayNeumannBC::computeQpResidual(RealEigenVector & residual)
{
  residual -= _test[_i][_qp] * _value;
}
