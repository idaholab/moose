//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowHeatAdvection.h"

registerMooseObject("PorousFlowApp", PorousFlowHeatAdvection);
registerMooseObject("PorousFlowApp", ADPorousFlowHeatAdvection);

template <bool is_ad>
InputParameters
PorousFlowHeatAdvectionTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowDarcyBaseTempl<is_ad>::validParams();
  params.addClassDescription("Fully-upwinded heat flux, advected by the fluid");
  return params;
}

template <bool is_ad>
PorousFlowHeatAdvectionTempl<is_ad>::PorousFlowHeatAdvectionTempl(
    const InputParameters & parameters)
  : PorousFlowDarcyBaseTempl<is_ad>(parameters),
    _enthalpy(this->template getGenericMaterialProperty<std::vector<Real>, is_ad>(
        "PorousFlow_fluid_phase_enthalpy_nodal")),
    _denthalpy_dvar(is_ad ? nullptr
                          : &this->template getMaterialProperty<std::vector<std::vector<Real>>>(
                                "dPorousFlow_fluid_phase_enthalpy_nodal_dvar")),
    _relative_permeability(this->template getGenericMaterialProperty<std::vector<Real>, is_ad>(
        "PorousFlow_relative_permeability_nodal")),
    _drelative_permeability_dvar(
        is_ad ? nullptr
              : &this->template getMaterialProperty<std::vector<std::vector<Real>>>(
                    "dPorousFlow_relative_permeability_nodal_dvar"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PorousFlowHeatAdvectionTempl<is_ad>::mobility(unsigned nodenum, unsigned phase) const
{
  return _enthalpy[nodenum][phase] * _fluid_density_node[nodenum][phase] *
         _relative_permeability[nodenum][phase] / _fluid_viscosity[nodenum][phase];
}

template <bool is_ad>
Real
PorousFlowHeatAdvectionTempl<is_ad>::dmobility(unsigned nodenum,
                                               unsigned phase,
                                               unsigned pvar) const
{
  if constexpr (!is_ad)
  {
    Real dm = (*_denthalpy_dvar)[nodenum][phase][pvar] * _fluid_density_node[nodenum][phase] *
              _relative_permeability[nodenum][phase] / _fluid_viscosity[nodenum][phase];
    dm += _enthalpy[nodenum][phase] * (*_dfluid_density_node_dvar)[nodenum][phase][pvar] *
          _relative_permeability[nodenum][phase] / _fluid_viscosity[nodenum][phase];
    dm += _enthalpy[nodenum][phase] * _fluid_density_node[nodenum][phase] *
          (*_drelative_permeability_dvar)[nodenum][phase][pvar] / _fluid_viscosity[nodenum][phase];
    dm -= _enthalpy[nodenum][phase] * _fluid_density_node[nodenum][phase] *
          _relative_permeability[nodenum][phase] * (*_dfluid_viscosity_dvar)[nodenum][phase][pvar] /
          std::pow(_fluid_viscosity[nodenum][phase], 2);
    return dm;
  }
  else
    libmesh_ignore(nodenum, phase, pvar);
  return 0.0;
}

template class PorousFlowHeatAdvectionTempl<false>;
template class PorousFlowHeatAdvectionTempl<true>;
