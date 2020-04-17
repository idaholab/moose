//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPresetBC.h"

registerMooseObjectDeprecated("MooseApp", ADPresetBC, "06/30/2020 24:00");

InputParameters
ADPresetBC::validParams()
{
  InputParameters params = ADDirichletBC::validParams();
  params.addClassDescription(
      "Similar to ADDirichletBC except the value is applied before the solve begins. Deprecated: "
      "use ADDirichletBC with preset = true instead.");

  // Utilize the new ADDirichletBC with preset, set true and don't let the user change it
  params.set<bool>("preset") = true;
  params.suppressParameter<bool>("preset");

  return params;
}

ADPresetBC::ADPresetBC(const InputParameters & parameters) : ADDirichletBC(parameters)
{
  mooseDeprecated("Use ADDirichletBC with preset = true instead of ADPresetBC");
}
