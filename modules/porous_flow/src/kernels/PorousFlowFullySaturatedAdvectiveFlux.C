//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFullySaturatedAdvectiveFlux.h"

registerMooseObject("PorousFlowApp", PorousFlowFullySaturatedAdvectiveFlux);

InputParameters
PorousFlowFullySaturatedAdvectiveFlux::validParams()
{
  InputParameters params = PorousFlowDarcyBase::validParams();
  params.addParam<unsigned int>(
      "fluid_component", 0, "The index corresponding to the fluid component for this kernel");
  params.addParam<bool>("multiply_by_density",
                        true,
                        "If true, then this Kernel is the fluid mass "
                        "flux.  If false, then this Kernel is the "
                        "fluid volume flux (which is common in "
                        "poro-mechanics)");
  params.addClassDescription("Fully-upwinded advective flux of the fluid component given by "
                             "fluid_component, in a single-phase fluid");
  return params;
}

PorousFlowFullySaturatedAdvectiveFlux::PorousFlowFullySaturatedAdvectiveFlux(
    const InputParameters & parameters)
  : PorousFlowDarcyBase(parameters),
    _mass_fractions(
        getMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_nodal")),
    _dmass_fractions_dvar(getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
        "dPorousFlow_mass_frac_nodal_dvar")),
    _fluid_component(getParam<unsigned int>("fluid_component")),
    _multiply_by_density(getParam<bool>("multiply_by_density"))
{
  if (_dictator.numPhases() != 1)
    mooseError(
        "PorousFlowFullySaturatedAdvectiveFlux should not be used for multi-phase scenarios as "
        "it does not include relative-permeability effects");
}

Real
PorousFlowFullySaturatedAdvectiveFlux::mobility(unsigned nodenum, unsigned phase) const
{
  if (_multiply_by_density == false)
    return _mass_fractions[nodenum][phase][_fluid_component] / _fluid_viscosity[nodenum][phase];
  return _mass_fractions[nodenum][phase][_fluid_component] * _fluid_density_node[nodenum][phase] /
         _fluid_viscosity[nodenum][phase];
}

Real
PorousFlowFullySaturatedAdvectiveFlux::dmobility(unsigned nodenum,
                                                 unsigned phase,
                                                 unsigned pvar) const
{
  if (_multiply_by_density == false)
    return _dmass_fractions_dvar[nodenum][phase][_fluid_component][pvar] /
               _fluid_viscosity[nodenum][phase] -
           _mass_fractions[nodenum][phase][_fluid_component] *
               _dfluid_viscosity_dvar[nodenum][phase][pvar] /
               Utility::pow<2>(_fluid_viscosity[nodenum][phase]);
  Real dm = _dmass_fractions_dvar[nodenum][phase][_fluid_component][pvar] *
            _fluid_density_node[nodenum][phase] / _fluid_viscosity[nodenum][phase];
  dm += _mass_fractions[nodenum][phase][_fluid_component] *
        _dfluid_density_node_dvar[nodenum][phase][pvar] / _fluid_viscosity[nodenum][phase];
  dm -= _mass_fractions[nodenum][phase][_fluid_component] * _fluid_density_node[nodenum][phase] *
        _dfluid_viscosity_dvar[nodenum][phase][pvar] /
        Utility::pow<2>(_fluid_viscosity[nodenum][phase]);
  return dm;
}
