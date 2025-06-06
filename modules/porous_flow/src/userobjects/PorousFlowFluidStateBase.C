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
  params.addParam<unsigned int>("liquid_phase_number", 0, "The phase number of the liquid phase");
  params.addRequiredParam<UserObjectName>("capillary_pressure",
                                          "Name of the UserObject defining the capillary pressure");
  params.addClassDescription("Base class for fluid state classes");
  return params;
}

PorousFlowFluidStateBase::PorousFlowFluidStateBase(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _aqueous_phase_number(getParam<unsigned int>("liquid_phase_number")),
    _R(8.3144598),
    _T_c2k(273.15),
    _pc(getUserObject<PorousFlowCapillaryPressure>("capillary_pressure"))
{
}

void
PorousFlowFluidStateBase::clearFluidStateProperties(std::vector<FluidStateProperties> & fsp) const
{
  std::fill(fsp.begin(), fsp.end(), _empty_fsp);
}
