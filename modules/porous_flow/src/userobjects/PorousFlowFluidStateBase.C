//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidStateBase.h"

template <>
InputParameters
validParams<PorousFlowFluidStateBase>()
{
  InputParameters params = validParams<PorousFlowFluidStateFlash>();
  params.addParam<unsigned int>("liquid_phase_number", 0, "The phase number of the liquid phase");
  params.addParam<unsigned int>(
      "liquid_fluid_component", 0, "The fluid component number of the liquid phase");
  params.addRequiredParam<UserObjectName>("capillary_pressure",
                                          "Name of the UserObject defining the capillary pressure");
  params.addClassDescription("Base class for fluid state classes");
  return params;
}

PorousFlowFluidStateBase::PorousFlowFluidStateBase(const InputParameters & parameters)
  : PorousFlowFluidStateFlash(parameters),
    _num_phases(2),
    _num_components(2),
    _aqueous_phase_number(getParam<unsigned int>("liquid_phase_number")),
    _gas_phase_number(1 - _aqueous_phase_number),
    _aqueous_fluid_component(getParam<unsigned int>("liquid_fluid_component")),
    _gas_fluid_component(1 - _aqueous_fluid_component),
    _R(8.3144598),
    _T_c2k(273.15),
    _nr_max_its(42),
    _nr_tol(1.0e-12),
    _pc_uo(getUserObject<PorousFlowCapillaryPressure>("capillary_pressure"))
{
  // Check that _aqueous_phase_number is <= total number of phases
  if (_aqueous_phase_number >= _num_phases)
    mooseError("The liquid_phase_number given in ", _name, " is larger than the number of phases");

  // Check that _aqueous_phase_number is <= total number of phases
  if (_aqueous_fluid_component >= _num_components)
    mooseError("The liquid_fluid_component given in ",
               _name,
               " is larger than the number of fluid components");
}

unsigned int
PorousFlowFluidStateBase::numPhases() const
{
  return _num_phases;
}

unsigned int
PorousFlowFluidStateBase::aqueousPhaseIndex() const
{
  return _aqueous_phase_number;
}

unsigned int
PorousFlowFluidStateBase::gasPhaseIndex() const
{
  return _gas_phase_number;
}

unsigned int
PorousFlowFluidStateBase::aqueousComponentIndex() const
{
  return _aqueous_fluid_component;
}

unsigned int
PorousFlowFluidStateBase::gasComponentIndex() const
{
  return _gas_fluid_component;
}

void
PorousFlowFluidStateBase::clearFluidStateProperties(std::vector<FluidStateProperties> & fsp) const
{
  std::fill(fsp.begin(), fsp.end(), FluidStateProperties(_num_components));
}
