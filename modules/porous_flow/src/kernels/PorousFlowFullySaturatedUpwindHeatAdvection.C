//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFullySaturatedUpwindHeatAdvection.h"

registerMooseObject("PorousFlowApp", PorousFlowFullySaturatedUpwindHeatAdvection);

InputParameters
PorousFlowFullySaturatedUpwindHeatAdvection::validParams()
{
  InputParameters params = PorousFlowDarcyBase::validParams();
  params.addClassDescription("Heat advection by a fluid.  The fluid is assumed to have a single "
                             "phase, and the advection is fully upwinded");
  return params;
}

PorousFlowFullySaturatedUpwindHeatAdvection::PorousFlowFullySaturatedUpwindHeatAdvection(
    const InputParameters & parameters)
  : PorousFlowDarcyBase(parameters),
    _enthalpy(getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_enthalpy_nodal")),
    _denthalpy_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_fluid_phase_enthalpy_nodal_dvar"))
{
  if (_dictator.numPhases() != 1)
    mooseError("PorousFlowFullySaturatedUpwindHeatAdvection should not be used for multi-phase "
               "scenarios as it does not include relative-permeability effects");
}

Real
PorousFlowFullySaturatedUpwindHeatAdvection::mobility(unsigned nodenum, unsigned phase) const
{
  return _enthalpy[nodenum][phase] * _fluid_density_node[nodenum][phase] /
         _fluid_viscosity[nodenum][phase];
}

Real
PorousFlowFullySaturatedUpwindHeatAdvection::dmobility(unsigned nodenum,
                                                       unsigned phase,
                                                       unsigned pvar) const
{
  Real dm = _denthalpy_dvar[nodenum][phase][pvar] * _fluid_density_node[nodenum][phase] /
            _fluid_viscosity[nodenum][phase];
  dm += _enthalpy[nodenum][phase] * _dfluid_density_node_dvar[nodenum][phase][pvar] /
        _fluid_viscosity[nodenum][phase];
  dm -= _enthalpy[nodenum][phase] * _fluid_density_node[nodenum][phase] *
        _dfluid_viscosity_dvar[nodenum][phase][pvar] /
        Utility::pow<2>(_fluid_viscosity[nodenum][phase]);
  return dm;
}
