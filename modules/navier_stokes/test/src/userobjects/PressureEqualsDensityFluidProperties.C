//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PressureEqualsDensityFluidProperties.h"

registerMooseObject("NavierStokesTestApp", PressureEqualsDensityFluidProperties);

InputParameters
PressureEqualsDensityFluidProperties::validParams()
{
  InputParameters params = IdealGasFluidProperties::validParams();
  return params;
}

PressureEqualsDensityFluidProperties::PressureEqualsDensityFluidProperties(
    const InputParameters & parameters)
  : IdealGasFluidProperties(parameters)
{
}

ADReal
PressureEqualsDensityFluidProperties::p_from_v_e(const ADReal & v, const ADReal & /*e*/) const
{
  return 1 / v;
}

ADReal
PressureEqualsDensityFluidProperties::T_from_v_e(const ADReal & /*v*/, const ADReal & /*e*/) const
{
  return 1;
}
