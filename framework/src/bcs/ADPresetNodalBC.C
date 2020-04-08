//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPresetNodalBC.h"
defineADLegacyParams(ADPresetNodalBC);

template <ComputeStage compute_stage>
InputParameters
ADPresetNodalBC<compute_stage>::validParams()
{
  InputParameters params = ADNodalBC<compute_stage>::validParams();

  params.addClassDescription(
      "Nodal boundary condition base class with preset solution vector values.");

  // Utilize the new ADDirichletBCBase with preset, set true and don't let the user change it
  params.set<bool>("preset") = true;
  params.suppressParameter<bool>("preset");

  return params;
}

template <ComputeStage compute_stage>
ADPresetNodalBC<compute_stage>::ADPresetNodalBC(const InputParameters & parameters)
  : ADDirichletBCBase<compute_stage>(parameters)
{
  mooseDeprecated("Inherit from ADDirichletBCBase with preset = true instead of ADPresetNodalBC");
}
