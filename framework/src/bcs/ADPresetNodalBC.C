//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPresetNodalBC.h"

InputParameters
ADPresetNodalBC::validParams()
{
  InputParameters params = ADNodalBC::validParams();

  params.addClassDescription(
      "Nodal boundary condition base class with preset solution vector values.");

  // Utilize the new ADDirichletBCBase with preset, set true and don't let the user change it
  params.set<bool>("preset") = true;
  params.suppressParameter<bool>("preset");

  return params;
}

ADPresetNodalBC::ADPresetNodalBC(const InputParameters & parameters) : ADDirichletBCBase(parameters)
{
  mooseDeprecated("Inherit from ADDirichletBCBase with preset = true instead of ADPresetNodalBC");
}
