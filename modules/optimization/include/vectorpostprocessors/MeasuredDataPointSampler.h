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
#include "PointValueSampler.h"

class MeasuredDataPointSampler : public PointValueSampler
{
public:
  static InputParameters validParams();

  MeasuredDataPointSampler(const InputParameters & parameters);

  virtual void finalize() override;

protected:
  std::vector<VectorPostprocessorValue *> _measured;
  std::vector<VectorPostprocessorValue *> _diff;
};
