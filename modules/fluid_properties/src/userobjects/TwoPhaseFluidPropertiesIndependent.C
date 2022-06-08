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

InputParameters
TwoPhaseFluidPropertiesIndependent::validParams()
{
  InputParameters params = TwoPhaseFluidProperties::validParams();

  params.addClassDescription(
      "2-phase fluid properties for 2 independent single-phase fluid properties");

  params.makeParamRequired<UserObjectName>("fp_liquid");
  params.makeParamRequired<UserObjectName>("fp_vapor");
  params.addParam<bool>(
      "error_on_unimplemented",
      true,
      "If true, throw an error when a 2-phase interface is called. Else, return a zero value.");

  return params;
}

TwoPhaseFluidPropertiesIndependent::TwoPhaseFluidPropertiesIndependent(
    const InputParameters & parameters)
  : TwoPhaseFluidProperties(parameters),

    _error_on_unimplemented(getParam<bool>("error_on_unimplemented"))
{
  _fp_liquid = &getUserObject<SinglePhaseFluidProperties>("fp_liquid");
  _fp_vapor = &getUserObject<SinglePhaseFluidProperties>("fp_vapor");
}

Real
TwoPhaseFluidPropertiesIndependent::getTwoPhaseInterfaceDummyValue() const
{
  if (_error_on_unimplemented)
    mooseError(
        "The 2-phase fluid properties class 'TwoPhaseFluidPropertiesIndependent' does not allow "
        "calling any 2-phase property interfaces.");
  else
    return 0;
}

Real
TwoPhaseFluidPropertiesIndependent::p_critical() const
{
  return getTwoPhaseInterfaceDummyValue();
}

Real
TwoPhaseFluidPropertiesIndependent::T_triple() const
{
  return getTwoPhaseInterfaceDummyValue();
}

Real TwoPhaseFluidPropertiesIndependent::T_sat(Real) const
{
  return getTwoPhaseInterfaceDummyValue();
}

DualReal
TwoPhaseFluidPropertiesIndependent::T_sat(const DualReal &) const
{
  return getTwoPhaseInterfaceDummyValue();
}

Real TwoPhaseFluidPropertiesIndependent::p_sat(Real) const
{
  return getTwoPhaseInterfaceDummyValue();
}

DualReal
TwoPhaseFluidPropertiesIndependent::p_sat(const DualReal &) const
{
  return getTwoPhaseInterfaceDummyValue();
}

Real TwoPhaseFluidPropertiesIndependent::dT_sat_dp(Real) const
{
  return getTwoPhaseInterfaceDummyValue();
}

Real
TwoPhaseFluidPropertiesIndependent::L_fusion() const
{
  return getTwoPhaseInterfaceDummyValue();
}

Real TwoPhaseFluidPropertiesIndependent::sigma_from_T(Real) const
{
  return getTwoPhaseInterfaceDummyValue();
}

DualReal
TwoPhaseFluidPropertiesIndependent::sigma_from_T(const DualReal &) const
{
  return getTwoPhaseInterfaceDummyValue();
}

Real TwoPhaseFluidPropertiesIndependent::dsigma_dT_from_T(Real) const
{
  return getTwoPhaseInterfaceDummyValue();
}
