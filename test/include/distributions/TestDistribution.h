//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Distribution.h"

class TestDistribution : public Distribution
{
public:
  static InputParameters validParams();

  TestDistribution(const InputParameters & parameters);

  virtual Real pdf(const Real & x) const override;
  virtual Real cdf(const Real & x) const override;
  virtual Real quantile(const Real & y) const override;
};
