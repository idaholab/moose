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

template <>
InputParameters
validParams<ArrayDirichletBC>()
{
  InputParameters p = validParams<ArrayNodalBC>();
  p.addRequiredParam<RealEigenVector>("values",
                                      "The values the components must take on the boundary");
  p.declareControllable("values");
  p.addClassDescription(
      "Imposes the essential boundary condition $\\vec{u}=\\vec{g}$, where $\\vec{g}$ "
      "are constant, controllable values.");
  return p;
}

ArrayDirichletBC::ArrayDirichletBC(const InputParameters & parameters)
  : ArrayNodalBC(parameters), _values(getParam<RealEigenVector>("values"))
{
}

RealEigenVector
ArrayDirichletBC::computeQpResidual()
{
  return _u - _values;
}
