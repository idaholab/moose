//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POINTVALUESAMPLER_H
#define POINTVALUESAMPLER_H

// MOOSE includes
#include "PointSamplerBase.h"

// Forward Declarations
class PointValueSampler;

template <>
InputParameters validParams<PointValueSampler>();

class PointValueSampler : public PointSamplerBase
{
public:
  PointValueSampler(const InputParameters & parameters);
};

#endif
