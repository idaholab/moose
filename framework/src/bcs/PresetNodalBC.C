//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PresetNodalBC.h"

defineLegacyParams(PresetNodalBC);

InputParameters
PresetNodalBC::validParams()
{
  InputParameters params = DirichletBCBase::validParams();

  // Utilize the new DirichletBC with preset, set true and don't let the user change it
  params.set<bool>("preset") = true;
  params.suppressParameter<bool>("preset");

  return params;
}

PresetNodalBC::PresetNodalBC(const InputParameters & parameters) : DirichletBCBase(parameters)
{
  mooseDeprecated(name(),
                  ": inherit from DirichletBCBase with preset = true instead of PresetNodalBC");
}
