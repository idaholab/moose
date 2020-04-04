//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestTwoPhaseFluidProperties.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("FluidPropertiesTestApp", TestTwoPhaseFluidProperties);

InputParameters
TestTwoPhaseFluidProperties::validParams()
{
  InputParameters params = TwoPhaseFluidProperties::validParams();

  params.addClassDescription("Test 2-phase fluid properties");

  params.makeParamRequired<UserObjectName>("fp_liquid");
  params.makeParamRequired<UserObjectName>("fp_vapor");

  return params;
}

TestTwoPhaseFluidProperties::TestTwoPhaseFluidProperties(const InputParameters & parameters)
  : TwoPhaseFluidProperties(parameters)
{
  _fp_liquid = &getUserObject<SinglePhaseFluidProperties>("fp_liquid");
  _fp_vapor = &getUserObject<SinglePhaseFluidProperties>("fp_vapor");
}

Real
TestTwoPhaseFluidProperties::p_critical() const
{
  return 25;
}

Real
TestTwoPhaseFluidProperties::T_sat(Real p) const
{
  return 2 * p;
}

Real
TestTwoPhaseFluidProperties::p_sat(Real T) const
{
  return 3 * T;
}

Real TestTwoPhaseFluidProperties::dT_sat_dp(Real /*p*/) const { return 2; }

Real
TestTwoPhaseFluidProperties::sigma_from_T(Real T) const
{
  return 5 * T;
}

Real TestTwoPhaseFluidProperties::dsigma_dT_from_T(Real /*T*/) const { return 5; }

bool
TestTwoPhaseFluidProperties::supportsPhaseChange() const
{
  return true;
}
