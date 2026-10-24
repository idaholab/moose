//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CheckNaNsTabulatedFluidProperties.h"

registerMooseObject("FluidPropertiesApp", CheckNaNsTabulatedFluidProperties);

InputParameters
CheckNaNsTabulatedFluidProperties::validParams()
{
  return TabulatedBicubicFluidProperties::validParams();
}

CheckNaNsTabulatedFluidProperties::CheckNaNsTabulatedFluidProperties(
    const InputParameters & parameters)
  : TabulatedBicubicFluidProperties(parameters)
{
}

void
CheckNaNsTabulatedFluidProperties::setSpecificVolumeGrid(const std::vector<Real> & v)
{
  _specific_volume = v;
  _v_min = v.front();
  _v_max = v.back();
}
