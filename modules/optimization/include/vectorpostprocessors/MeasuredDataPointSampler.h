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
#include "MeasuredDataPointSamplerBase.h"

// Forward Declarations
class MeasuredDataPointSampler;

template <>
InputParameters validParams<MeasuredDataPointSampler>();

// FIXME LYNN  This is an exact copy of PointSampler because I need to add measurement data to the
// values so that it can be sorted with the values.  This is all because SamplerBase has protected
// inheritance in PointSamplerBase.

class MeasuredDataPointSampler : public MeasuredDataPointSamplerBase
{
public:
  static InputParameters validParams();

  MeasuredDataPointSampler(const InputParameters & parameters);

  virtual void initialize() override;
};
