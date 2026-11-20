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
  // The fluid component indices are fixed by each derived fluid state class. These deprecated
  // parameters are retained so that existing input that sets them to their defaults still runs.
  const std::string deprecate_msg =
      "The fluid component indices are now fixed by each fluid state class, so this parameter can "
      "only take its default value. It is deprecated and will be removed on 2027-04-26; see the "
      "fluid state documentation for details.";
  params.addDeprecatedParam<unsigned int>(
      "liquid_fluid_component",
      0,
      "The fluid component number of the primary liquid component",
      deprecate_msg);
  params.addDeprecatedParam<unsigned int>(
      "salt_component", 2, "The component number of salt", deprecate_msg);
  params.addParam<bool>("compute_internal_energy", true, "Compute the fluid internal energy");
  params.addParam<bool>("compute_enthalpy", true, "Compute the fluid enthalpy");
  params.addClassDescription("Base class for multiple component fluid state classes");
  return params;
}

PorousFlowFluidStateMultiComponentBase::PorousFlowFluidStateMultiComponentBase(
    const InputParameters & parameters)
  : PorousFlowFluidStateFlash(parameters),
    _aqueous_fluid_component(0),
    _salt_component(libMesh::invalid_uint),
    _compute_internal_energy(getParam<bool>("compute_internal_energy")),
    _compute_enthalpy(getParam<bool>("compute_enthalpy"))
{
  // The component numbering is fixed, so reject any other value rather than silently ignore it
  if (getParam<unsigned int>("liquid_fluid_component") != _aqueous_fluid_component)
    paramError(
        "liquid_fluid_component",
        "Water is now always fluid component 0. Remove this parameter and renumber the fluid "
        "components in the rest of the input file to match");
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
