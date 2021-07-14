//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestConservedVarFluidProperties.h"

registerMooseObject("NavierStokesTestApp", TestConservedVarFluidProperties);

InputParameters
TestConservedVarFluidProperties::validParams()
{
  InputParameters params = IdealGasFluidProperties::validParams();
  return params;
}

TestConservedVarFluidProperties::TestConservedVarFluidProperties(const InputParameters & parameters)
  : IdealGasFluidProperties(parameters)
{
}

ADReal
TestConservedVarFluidProperties::p_from_v_e(const ADReal & v, const ADReal & /*e*/) const
{
  return 1 / v;
}

ADReal
TestConservedVarFluidProperties::T_from_v_e(const ADReal & /*v*/, const ADReal & /*e*/) const
{
  return 1;
}
