//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADArrayRobinBC.h"

registerMooseObject("MooseApp", ADArrayRobinBC);

InputParameters
ADArrayRobinBC::validParams()
{
  InputParameters params = ADArrayIntegratedBC::validParams();
  params.addParam<RealEigenVector>("value", "The prescribed flux $h$ of each component");
  params.addParam<RealEigenVector>("alpha", "The Robin coefficient $\\alpha$ of each component");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\nabla u \\cdot \\vec{n} = h - \\alpha u$ on every component of an "
                             "array variable, using automatic differentiation.");
  return params;
}

ADArrayRobinBC::ADArrayRobinBC(const InputParameters & parameters)
  : ADArrayIntegratedBC(parameters),
    _value(isParamValid("value") ? getParam<RealEigenVector>("value")
                                 : RealEigenVector::Zero(_count)),
    _alpha(isParamValid("alpha") ? getParam<RealEigenVector>("alpha")
                                 : RealEigenVector::Zero(_count))
{
  if (_value.size() != _count)
    paramError("value", "Must have as many entries as the variable has components");
  if (_alpha.size() != _count)
    paramError("alpha", "Must have as many entries as the variable has components");
}

void
ADArrayRobinBC::computeQpResidual(ADRealEigenVector & residual)
{
  residual = (_alpha.cwiseProduct(_u[_qp]) - _value) * _test[_i][_qp];
}
