//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionPresetBC.h"

registerMooseObjectDeprecated("MooseApp", FunctionPresetBC, "06/30/2020 24:00");

defineLegacyParams(FunctionPresetBC);

InputParameters
FunctionPresetBC::validParams()
{
  InputParameters params = FunctionDirichletBC::validParams();
  params.addClassDescription(
      "The same as FunctionDirichletBC except the value is applied before the solve begins. "
      "Deprecated: use FunctionDirichletBC with preset = true instead.");

  // Utilize the new FunctionDirichletBC with preset, set true and don't let the user change it
  params.set<bool>("preset") = true;
  params.suppressParameter<bool>("preset");

  return params;
}

FunctionPresetBC::FunctionPresetBC(const InputParameters & parameters)
  : FunctionDirichletBC(parameters)
{
  mooseDeprecated(
      name(), ": use FunctionDirichletBC with preset = true (default) instead of FunctionPresetBC");
}
