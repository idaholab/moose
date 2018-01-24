//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "LineMaterialRealSampler.h"

template <>
InputParameters
validParams<LineMaterialRealSampler>()
{
  InputParameters params = validParams<LineMaterialSamplerBase<Real>>();
  return params;
}

LineMaterialRealSampler::LineMaterialRealSampler(const InputParameters & parameters)
  : LineMaterialSamplerBase<Real>(parameters)
{
}

Real
LineMaterialRealSampler::getScalarFromProperty(const Real & property, const Point & /*curr_point*/)
{
  return property;
}
