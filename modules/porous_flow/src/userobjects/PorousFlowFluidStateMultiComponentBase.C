//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidStateMultiComponentBase.h"

InputParameters
PorousFlowFluidStateMultiComponentBase::validParams()
{
  InputParameters params = PorousFlowFluidStateFlash::validParams();
  params.addParam<unsigned int>(
      "liquid_fluid_component", 0, "The fluid component number of the primary liquid component");
  params.addParam<unsigned int>("salt_component", 2, "The component number of salt");
  params.addClassDescription("Base class for multiple component fluid state classes");
  return params;
}

PorousFlowFluidStateMultiComponentBase::PorousFlowFluidStateMultiComponentBase(
    const InputParameters & parameters)
  : PorousFlowFluidStateFlash(parameters),
    _aqueous_fluid_component(getParam<unsigned int>("liquid_fluid_component")),
    _salt_component(getParam<unsigned int>("salt_component")),
    _pidx(0),
    _Zidx(1),
    _Tidx(2),
    _Xidx(3)
{
}

void
PorousFlowFluidStateMultiComponentBase::phaseState(Real Zi,
                                                   Real Xi,
                                                   Real Yi,
                                                   FluidStatePhaseEnum & phase_state) const
{
  if (Zi <= Xi)
  {
    // In this case, there is not enough component i to form a gas phase,
    // so only a liquid phase is present
    phase_state = FluidStatePhaseEnum::LIQUID;
  }
  else if (Zi > Xi && Zi < Yi)
  {
    // Two phases are present
    phase_state = FluidStatePhaseEnum::TWOPHASE;
  }
  else // (Zi >= Yi)
  {
    // In this case, there is not enough water to form a liquid
    // phase, so only a gas phase is present
    phase_state = FluidStatePhaseEnum::GAS;
  }
}
