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
  params.addParam<int>(
      "output_invalid_value_as", -1, "Convert the invalid value index for output purposes");
  return params;
}

MeshDivisionAux::MeshDivisionAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _mesh_division(
        _c_fe_problem.getMeshDivision(getParam<MeshDivisionName>("mesh_division"), _tid)),
    _invalid_bin_value(getParam<int>("output_invalid_value_as"))
{
  // Check family types for reasonable choices
  if (!_var.isNodal() && _var.feType().order != CONSTANT)
    paramError("Variable order " + Moose::stringify(_var.feType().order) + " unsupported");
}

Real
MeshDivisionAux::computeValue()
{
  const auto value = isNodal() ? _mesh_division.divisionIndex(*_current_node)
                               : _mesh_division.divisionIndex(*_current_elem);
  if (value == MooseMeshDivision::INVALID_DIVISION_INDEX)
    return _invalid_bin_value;
  else
    return value;
}
