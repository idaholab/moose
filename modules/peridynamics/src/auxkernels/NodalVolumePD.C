//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalVolumePD.h"
#include "PeridynamicsMesh.h"

registerMooseObject("PeridynamicsApp", NodalVolumePD);

InputParameters
NodalVolumePD::validParams()
{
  InputParameters params = AuxKernelBasePD::validParams();
  params.addClassDescription("Class for output nodal area(2D) or nodal volume(3D)");

  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;

  return params;
}

NodalVolumePD::NodalVolumePD(const InputParameters & parameters) : AuxKernelBasePD(parameters)
{
  if (!_var.isNodal())
    mooseError("NodalVolumePD operates on nodal variable!");
}

Real
NodalVolumePD::computeValue()
{
  return _pdmesh.getNodeVolume(_current_node->id());
}
