//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayDirichletBC.h"

registerMooseObject("MooseApp", ArrayDirichletBC);

InputParameters
ArrayDirichletBC::validParams()
{
  InputParameters params = ArrayNodalBC::validParams();
  params.addRequiredParam<RealEigenVector>("values",
                                           "The values the components must take on the boundary");
  params.declareControllable("values");
  params.addClassDescription(
      "Imposes the essential boundary condition $\\vec{u}=\\vec{g}$, where $\\vec{g}$ "
      "are constant, controllable values.");
  return params;
}

ArrayDirichletBC::ArrayDirichletBC(const InputParameters & parameters)
  : ArrayNodalBC(parameters), _values(getParam<RealEigenVector>("values"))
{
  if (_values.size() != _count)
    paramError(
        "values", "Number of 'values' must equal number of variable components (", _count, ").");
}

void
ArrayDirichletBC::computeQpResidual(RealEigenVector & residual)
{
  residual = _u - _values;
}
