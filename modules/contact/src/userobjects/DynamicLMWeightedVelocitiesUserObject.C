//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DynamicLMWeightedVelocitiesUserObject.h"

registerMooseObject("ContactApp", DynamicLMWeightedVelocitiesUserObject);

InputParameters
DynamicLMWeightedVelocitiesUserObject::validParams()
{
  InputParameters params = LMWeightedVelocitiesUserObject::validParams();
  params.set<bool>("use_nodal_normal_derivatives") = false;
  params.set<bool>("ghost_point_neighbors") = false;
  params.addClassDescription(
      "Provides weighted-velocity data with frozen nodal geometry for dynamic mortar contact.");
  return params;
}

DynamicLMWeightedVelocitiesUserObject::DynamicLMWeightedVelocitiesUserObject(
    const InputParameters & parameters)
  : WeightedGapUserObject(parameters), LMWeightedVelocitiesUserObject(parameters)
{
}
