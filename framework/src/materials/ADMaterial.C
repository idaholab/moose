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

template <>
InputParameters
validParams<ADMaterial<RESIDUAL>>()
{
  InputParameters params = validParams<Material>();
  return params;
}
template <>
InputParameters
validParams<ADMaterial<JACOBIAN>>()
{
  return validParams<ADMaterial<RESIDUAL>>();
}

template <ComputeStage compute_stage>
ADMaterial<compute_stage>::ADMaterial(const InputParameters & parameters) : Material(parameters)
{
}

// explicit instantiation is required for AD base classes
adBaseClass(ADMaterial);
