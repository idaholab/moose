//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DynamicLMWeightedGapUserObject.h"

registerMooseObject("ContactApp", DynamicLMWeightedGapUserObject);

InputParameters
DynamicLMWeightedGapUserObject::validParams()
{
  InputParameters params = LMWeightedGapUserObject::validParams();
  params.set<bool>("use_nodal_normal_derivatives") = false;
  params.set<bool>("ghost_point_neighbors") = false;
  params.addClassDescription(
      "Provides weighted-gap data with frozen nodal normals for dynamic mortar contact.");
  return params;
}

DynamicLMWeightedGapUserObject::DynamicLMWeightedGapUserObject(const InputParameters & parameters)
  : WeightedGapUserObject(parameters), LMWeightedGapUserObject(parameters)
{
}
