//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FNSFHeatSource.h"
#include "FNSFUtils.h"

registerMooseObject("FusionApp", FNSFHeatSource);

InputParameters
FNSFHeatSource::validParams()
{
  InputParameters params = FNSFSource::validParams();
  params.addRequiredParam<std::vector<Real>>("heat", "Heat generation values for each cell.");
  return params;
}

FNSFHeatSource::FNSFHeatSource(const InputParameters & parameters) : FNSFSource(parameters)

{
  _source = getParam<std::vector<Real>>("heat");
}
