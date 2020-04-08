//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PresetBC.h"

registerMooseObjectDeprecated("MooseApp", PresetBC, "06/30/2020 24:00");

defineLegacyParams(PresetBC);

InputParameters
PresetBC::validParams()
{
  InputParameters params = DirichletBC::validParams();
  params.addClassDescription("Similar to DirichletBC except the value is applied before the solve "
                             "begins. Deprecated: use DirichletBC with preset = true instead.");

  // Utilize the new DirichletBC with preset, set true and don't let the user change it
  params.set<bool>("preset") = true;
  params.suppressParameter<bool>("preset");

  return params;
}

PresetBC::PresetBC(const InputParameters & parameters) : DirichletBC(parameters)
{
  mooseDeprecated(name(), ": use DirichletBC with preset = true instead of PresetBC");
}

Real
PresetBC::computeQpValue()
{
  return _value;
}
