//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshDivisionAux.h"
#include "MeshDivision.h"

registerMooseObject("MooseApp", MeshDivisionAux);

InputParameters
MeshDivisionAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Returns the value of the mesh division index for each element / node");
  params.addRequiredParam<MeshDivisionName>("mesh_division",
                                            "The mesh division providing the value");
  return params;
}

MeshDivisionAux::MeshDivisionAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _mesh_division(_c_fe_problem.getMeshDivision(getParam<MeshDivisionName>("mesh_division")))
{
  // Check family types for reasonable choices
  if (isNodal() && _var.feType().order != FIRST)
    paramError("Variable order " + Moose::stringify(_var.feType().order) +
               " unsupported for nodal variable");
  else if (_var.feType().order != CONSTANT)
    paramError("Variable order " + Moose::stringify(_var.feType().order) + " unsupported");
}

Real
MeshDivisionAux::computeValue()
{
  if (isNodal())
    return _mesh_division.divisionIndex(*_current_node);
  else
    return _mesh_division.divisionIndex(*_current_elem);
}
