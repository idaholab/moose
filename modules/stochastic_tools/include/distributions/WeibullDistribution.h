//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Weibull.h"

/**
 * A deprecated wrapper class used to generate a Weibull distribution
 */
class WeibullDistribution : public Weibull
{
public:
  static InputParameters validParams();
  WeibullDistribution(const InputParameters & parameters);
};
