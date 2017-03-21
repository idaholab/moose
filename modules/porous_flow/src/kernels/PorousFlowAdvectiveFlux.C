/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowAdvectiveFlux.h"

template <>
InputParameters
validParams<PorousFlowAdvectiveFlux>()
{
  InputParameters params = validParams<PorousFlowDarcyBase>();
  params.addParam<unsigned int>(
      "fluid_component", 0, "The index corresponding to the fluid component for this kernel");
  params.addClassDescription(
      "Fully-upwinded advective flux of the component given by fluid_component");
  return params;
}

PorousFlowAdvectiveFlux::PorousFlowAdvectiveFlux(const InputParameters & parameters)
  : PorousFlowDarcyBase(parameters),
    _mass_fractions(
        getMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_nodal")),
    _dmass_fractions_dvar(getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
        "dPorousFlow_mass_frac_nodal_dvar")),
    _relative_permeability(
        getMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_nodal")),
    _drelative_permeability_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_relative_permeability_nodal_dvar")),
    _fluid_component(getParam<unsigned int>("fluid_component"))
{
}

Real
PorousFlowAdvectiveFlux::mobility(unsigned nodenum, unsigned phase) const
{
  return _mass_fractions[nodenum][phase][_fluid_component] * _fluid_density_node[nodenum][phase] *
         _relative_permeability[nodenum][phase] / _fluid_viscosity[nodenum][phase];
}

Real
PorousFlowAdvectiveFlux::dmobility(unsigned nodenum, unsigned phase, unsigned pvar) const
{
  Real dm = _dmass_fractions_dvar[nodenum][phase][_fluid_component][pvar] *
            _fluid_density_node[nodenum][phase] * _relative_permeability[nodenum][phase] /
            _fluid_viscosity[nodenum][phase];
  dm += _mass_fractions[nodenum][phase][_fluid_component] *
        _dfluid_density_node_dvar[nodenum][phase][pvar] * _relative_permeability[nodenum][phase] /
        _fluid_viscosity[nodenum][phase];
  dm += _mass_fractions[nodenum][phase][_fluid_component] * _fluid_density_node[nodenum][phase] *
        _drelative_permeability_dvar[nodenum][phase][pvar] / _fluid_viscosity[nodenum][phase];
  dm -= _mass_fractions[nodenum][phase][_fluid_component] * _fluid_density_node[nodenum][phase] *
        _relative_permeability[nodenum][phase] * _dfluid_viscosity_dvar[nodenum][phase][pvar] /
        std::pow(_fluid_viscosity[nodenum][phase], 2);
  return dm;
}
