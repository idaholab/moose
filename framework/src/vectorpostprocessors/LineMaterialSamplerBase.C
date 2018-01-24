//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LineMaterialSamplerBase.h"

template <>
InputParameters
validParams<LineMaterialSamplerBase<Real>>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();
  params += validParams<SamplerBase>();
  params += validParams<BlockRestrictable>();
  params.addRequiredParam<Point>("start", "The beginning of the line");
  params.addRequiredParam<Point>("end", "The end of the line");
  params.addRequiredParam<std::vector<std::string>>(
      "property", "Name of the material property to be output along a line");

  return params;
}
