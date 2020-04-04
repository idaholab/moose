//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryOffsetPD.h"
#include "PeridynamicsMesh.h"

registerMooseObject("PeridynamicsApp", BoundaryOffsetPD);

InputParameters
BoundaryOffsetPD::validParams()
{
  InputParameters params = AuxKernelBasePD::validParams();
  params.addClassDescription(
      "Class for output offset of PD boundary nodes compared to initial FE mesh");

  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;

  return params;
}

BoundaryOffsetPD::BoundaryOffsetPD(const InputParameters & parameters) : AuxKernelBasePD(parameters)
{
  if (!_var.isNodal())
    mooseError("BoundaryOffsetPD operates on nodal variable!");
}

Real
BoundaryOffsetPD::computeValue()
{
  return _pdmesh.getBoundaryOffset(_current_node->id());
}
