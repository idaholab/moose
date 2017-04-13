/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowHeatAdvection.h"

template <>
InputParameters
validParams<PorousFlowHeatAdvection>()
{
  InputParameters params = validParams<PorousFlowDarcyBase>();
  params.addClassDescription("Fully-upwinded heat flux, advected by the fluid");
  return params;
}

PorousFlowHeatAdvection::PorousFlowHeatAdvection(const InputParameters & parameters)
  : PorousFlowDarcyBase(parameters),
    _enthalpy(getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_enthalpy_nodal")),
    _denthalpy_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_fluid_phase_enthalpy_nodal_dvar")),
    _relative_permeability(
        getMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_nodal")),
    _drelative_permeability_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_relative_permeability_nodal_dvar"))
{
}

Real
PorousFlowHeatAdvection::mobility(unsigned nodenum, unsigned phase) const
{
  return _enthalpy[nodenum][phase] * _fluid_density_node[nodenum][phase] *
         _relative_permeability[nodenum][phase] / _fluid_viscosity[nodenum][phase];
}

Real
PorousFlowHeatAdvection::dmobility(unsigned nodenum, unsigned phase, unsigned pvar) const
{
  Real dm = _denthalpy_dvar[nodenum][phase][pvar] * _fluid_density_node[nodenum][phase] *
            _relative_permeability[nodenum][phase] / _fluid_viscosity[nodenum][phase];
  dm += _enthalpy[nodenum][phase] * _dfluid_density_node_dvar[nodenum][phase][pvar] *
        _relative_permeability[nodenum][phase] / _fluid_viscosity[nodenum][phase];
  dm += _enthalpy[nodenum][phase] * _fluid_density_node[nodenum][phase] *
        _drelative_permeability_dvar[nodenum][phase][pvar] / _fluid_viscosity[nodenum][phase];
  dm -= _enthalpy[nodenum][phase] * _fluid_density_node[nodenum][phase] *
        _relative_permeability[nodenum][phase] * _dfluid_viscosity_dvar[nodenum][phase][pvar] /
        std::pow(_fluid_viscosity[nodenum][phase], 2);
  return dm;
}
