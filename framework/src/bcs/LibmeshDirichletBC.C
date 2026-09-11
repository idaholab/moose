//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LibmeshDirichletBC.h"

registerMooseObject("MooseApp", LibmeshDirichletBC);

InputParameters
LibmeshDirichletBC::validParams()
{
  InputParameters params = LibmeshDirichletBCBase::validParams();
  params.addRequiredParam<Real>("value", "Value of the BC");
  params.addClassDescription(
      "Imposes the essential boundary condition $u=g$, where $g$ is a constant, through libMesh's "
      "constraint system, which projects $g$ onto the variable's boundary trace space.");
  return params;
}

LibmeshDirichletBC::LibmeshDirichletBC(const InputParameters & parameters)
  : LibmeshDirichletBCBase(parameters), _value(getParam<Real>("value"))
{
}

Real
LibmeshDirichletBC::value(const libMesh::Point & /*p*/, Real /*time*/) const
{
  return _value;
}
