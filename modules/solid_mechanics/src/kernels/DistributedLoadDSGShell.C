//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DistributedLoadDSGShell.h"
#include "Function.h"

registerMooseObject("SolidMechanicsApp", DistributedLoadDSGShell);

InputParameters
DistributedLoadDSGShell::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Applies a load per unit area of the mid-surface of a DSG shell element, either along the "
      "axis of the displacement variable (self weight, wind) or along the element normal "
      "(pressure), with the sign convention of ADDistributedLoadShell and of the Pressure "
      "boundary condition: a positive value acts against the axis, or against the normal.");
  params.addRequiredCoupledVar(
      "displacements",
      "The three displacement variables of the shell nodes, in the order of the global axes X, Y "
      "and Z, for example 'disp_x disp_y disp_z'. The kernel finds the axis of its own variable "
      "in this list.");
  params.addRequiredParam<FunctionName>(
      "function",
      "The load per unit area of the shell mid-surface, as a function of time and position; a "
      "constant such as '90' is a valid function. A positive value acts against the axis of the "
      "variable or, with project_load_to_normal, against the element normal (a self weight on "
      "a horizontal plate is a positive function on the vertical displacement), the convention "
      "of ADDistributedLoadShell and of the Pressure boundary condition.");
  params.addParam<bool>(
      "project_load_to_normal",
      false,
      "False by default: the load acts along the global axis of the variable of this kernel, as a "
      "self weight does on the vertical displacement. Set to true for a pressure: the load acts "
      "against the element normal and this kernel applies the component of it along its own "
      "axis, so three kernels with the same function, one per displacement variable, are "
      "needed. The normal is the one of the element frame, which follows the node ordering of "
      "the element (right-hand rule), so a positive pressure on a mesh whose normals point into "
      "a closed surface is an internal pressure.");
  params.set<bool>("use_displaced_mesh") = false;
  return params;
}

DistributedLoadDSGShell::DistributedLoadDSGShell(const InputParameters & parameters)
  : Kernel(parameters),
    _component(displacementComponent()),
    _function(getFunction("function")),
    _project_load_to_normal(getParam<bool>("project_load_to_normal")),
    _rotation_matrix(getMaterialProperty<RankTwoTensor>("dsg_shell_rotation_matrix"))
{
}

unsigned int
DistributedLoadDSGShell::displacementComponent()
{
  if (coupledComponents("displacements") != 3)
    paramError("displacements", "'displacements' must list three variables.");

  for (unsigned int i = 0; i < 3; ++i)
    if (coupled("displacements", i) == _var.number())
      return i;

  paramError("variable", "The variable of this kernel must be one of the 'displacements'.");
}

Real
DistributedLoadDSGShell::computeQpResidual()
{
  Real load = _function.value(_t, _q_point[_qp]);

  // The element normal is the third row of the rotation matrix of the element frame
  if (_project_load_to_normal)
    load *= _rotation_matrix[_qp](2, _component);

  // The sign convention of ADDistributedLoadShell and of the Pressure boundary condition: a
  // positive value acts against the axis, or against the normal
  return _test[_i][_qp] * load;
}
