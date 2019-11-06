//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ADMaterial.h"

defineADLegacyParams(ADMaterial);

template <ComputeStage compute_stage>
InputParameters
ADMaterial<compute_stage>::validParams()
{
  InputParameters params = Material::validParams();
  return params;
}

template <ComputeStage compute_stage>
ADMaterial<compute_stage>::ADMaterial(const InputParameters & parameters) : Material(parameters)
{
}

// explicit instantiation is required for AD base classes
adBaseClass(ADMaterial);
