//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidStateBase.h"

InputParameters
PorousFlowFluidStateBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  // The liquid phase is always phase 0 and the gas phase is always phase 1. This deprecated
  // parameter is retained so that existing input that sets it to 0 still runs.
  params.addDeprecatedParam<unsigned int>(
      "liquid_phase_number",
      0,
      "The phase number of the liquid phase",
      "The liquid phase is always phase 0 and the gas phase is phase 1, so this parameter can only "
      "be 0. It is deprecated and will be removed on 2027-04-26; see the fluid state "
      "documentation for details.");
  params.addRequiredParam<UserObjectName>("capillary_pressure",
                                          "Name of the UserObject defining the capillary pressure");
  params.addClassDescription("Base class for fluid state classes");
  return params;
}

PorousFlowFluidStateBase::PorousFlowFluidStateBase(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _aqueous_phase_number(0),
    _gas_phase_number(1),
    _R(8.3144598),
    _T_c2k(273.15),
    _pc(getUserObject<PorousFlowCapillaryPressure>("capillary_pressure"))
{
  // The phase numbering is fixed, so reject any other value rather than silently ignore it
  if (getParam<unsigned int>("liquid_phase_number") != _aqueous_phase_number)
    paramError("liquid_phase_number",
               "The liquid phase is now always phase 0 and the gas phase is phase 1. Remove this "
               "parameter and renumber the phases in the rest of the input file to match");
}

void
PorousFlowFluidStateBase::clearFluidStateProperties(std::vector<FluidStateProperties> & fsp) const
{
  std::fill(fsp.begin(), fsp.end(), _empty_fsp);
}
