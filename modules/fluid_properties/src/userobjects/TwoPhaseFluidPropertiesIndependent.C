//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TwoPhaseFluidPropertiesIndependent.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("FluidPropertiesApp", TwoPhaseFluidPropertiesIndependent);

template <>
InputParameters
validParams<TwoPhaseFluidPropertiesIndependent>()
{
  InputParameters params = validParams<TwoPhaseFluidProperties>();

  params.addClassDescription(
      "2-phase fluid properties for 2 independent single-phase fluid properties");

  params.makeParamRequired<UserObjectName>("fp_liquid");
  params.makeParamRequired<UserObjectName>("fp_vapor");

  return params;
}

TwoPhaseFluidPropertiesIndependent::TwoPhaseFluidPropertiesIndependent(
    const InputParameters & parameters)
  : TwoPhaseFluidProperties(parameters)

{
  _fp_liquid = &getUserObject<SinglePhaseFluidProperties>("fp_liquid");
  _fp_vapor = &getUserObject<SinglePhaseFluidProperties>("fp_vapor");
}

[[noreturn]] void
TwoPhaseFluidPropertiesIndependent::throwNotImplementedError() const
{
  mooseError(
      name(),
      ": The 2-phase fluid properties class 'TwoPhaseFluidPropertiesIndependent' does not allow "
      "calling any 2-phase property interfaces.");
}

Real
TwoPhaseFluidPropertiesIndependent::p_critical() const
{
  throwNotImplementedError();
}

Real TwoPhaseFluidPropertiesIndependent::T_sat(Real) const { throwNotImplementedError(); }

Real TwoPhaseFluidPropertiesIndependent::p_sat(Real) const { throwNotImplementedError(); }

Real TwoPhaseFluidPropertiesIndependent::dT_sat_dp(Real) const { throwNotImplementedError(); }
