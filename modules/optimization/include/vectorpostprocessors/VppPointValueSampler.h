//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "PointSamplerBase.h"

class VppPointValueSampler : public PointSamplerBase
{
public:
  static InputParameters validParams();

  VppPointValueSampler(const InputParameters & parameters);

  virtual void initialize() override;

private:
  /// helper function to read in data
  std::vector<Real> getPointDataHelper(const std::string & param);
};
