//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "PointVariableSamplerBase.h"

class PointValueSampler : public PointVariableSamplerBase
{
public:
  static InputParameters validParams();

  PointValueSampler(const InputParameters & parameters);
};
