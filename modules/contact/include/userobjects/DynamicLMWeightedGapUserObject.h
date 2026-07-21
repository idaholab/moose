//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LMWeightedGapUserObject.h"

/**
 * Weighted-gap data for dynamic mortar contact, which retains frozen nodal normals.
 */
class DynamicLMWeightedGapUserObject : public LMWeightedGapUserObject
{
public:
  static InputParameters validParams();

  DynamicLMWeightedGapUserObject(const InputParameters & parameters);
};
