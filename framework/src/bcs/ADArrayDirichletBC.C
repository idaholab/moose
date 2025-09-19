//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADArrayDirichletBC.h"

registerMooseObject("MooseApp", ADArrayDirichletBC);

InputParameters
ADArrayDirichletBC::validParams()
{
  InputParameters params = ADArrayNodalBC::validParams();
  params.addRequiredParam<RealEigenVector>("values",
                                           "The values the components must take on the boundary");
  params.declareControllable("values");
  params.addClassDescription(
      "Imposes the essential boundary condition $\\vec{u}=\\vec{g}$, where $\\vec{g}$ "
      "are constant, controllable values.");
  return params;
}

ADArrayDirichletBC::ADArrayDirichletBC(const InputParameters & parameters)
  : ADArrayNodalBC(parameters), _values(getParam<RealEigenVector>("values"))
{
  if (_values.size() != _var.count())
    paramError("values",
               "Number of 'values' must equal number of variable components (",
               _var.count(),
               ").");
}

ADRealEigenVector
ADArrayDirichletBC::computeQpResidual()
{
  return _u - _values;
}
