//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowAdvectiveFlux.h"

registerMooseObject("PorousFlowApp", PorousFlowAdvectiveFlux);
registerMooseObject("PorousFlowApp", ADPorousFlowAdvectiveFlux);

template <bool is_ad>
InputParameters
PorousFlowAdvectiveFluxTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowDarcyBaseTempl<is_ad>::validParams();
  params.addParam<unsigned int>(
      "fluid_component", 0, "The index corresponding to the fluid component for this kernel");
  params.addClassDescription(
      "Fully-upwinded advective flux of the component given by fluid_component");
  return params;
}

template <bool is_ad>
PorousFlowAdvectiveFluxTempl<is_ad>::PorousFlowAdvectiveFluxTempl(
    const InputParameters & parameters)
  : PorousFlowDarcyBaseTempl<is_ad>(parameters),
    _mass_fractions(
        this->template getGenericMaterialProperty<std::vector<std::vector<Real>>, is_ad>(
            "PorousFlow_mass_frac_nodal")),
    _dmass_fractions_dvar(
        is_ad ? nullptr
              : &this->template getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
                    "dPorousFlow_mass_frac_nodal_dvar")),
    _relative_permeability(this->template getGenericMaterialProperty<std::vector<Real>, is_ad>(
        "PorousFlow_relative_permeability_nodal")),
    _drelative_permeability_dvar(
        is_ad ? nullptr
              : &this->template getMaterialProperty<std::vector<std::vector<Real>>>(
                    "dPorousFlow_relative_permeability_nodal_dvar")),
    _fluid_component(this->template getParam<unsigned int>("fluid_component"))
{
  if (_fluid_component >= _dictator.numComponents())
    this->paramError(
        "fluid_component",
        "The Dictator proclaims that the maximum fluid component index in this simulation is ",
        _dictator.numComponents() - 1,
        " whereas you have used ",
        _fluid_component,
        ". Remember that indexing starts at 0. The Dictator does not take such mistakes lightly.");
}

template <bool is_ad>
GenericReal<is_ad>
PorousFlowAdvectiveFluxTempl<is_ad>::mobility(unsigned nodenum, unsigned phase) const
{
  return _mass_fractions[nodenum][phase][_fluid_component] * _fluid_density_node[nodenum][phase] *
         _relative_permeability[nodenum][phase] / _fluid_viscosity[nodenum][phase];
}

template <bool is_ad>
Real
PorousFlowAdvectiveFluxTempl<is_ad>::dmobility(unsigned nodenum,
                                               unsigned phase,
                                               unsigned pvar) const
{
  if constexpr (!is_ad)
  {
    Real dm = (*_dmass_fractions_dvar)[nodenum][phase][_fluid_component][pvar] *
              _fluid_density_node[nodenum][phase] * _relative_permeability[nodenum][phase] /
              _fluid_viscosity[nodenum][phase];
    dm += _mass_fractions[nodenum][phase][_fluid_component] *
          (*_dfluid_density_node_dvar)[nodenum][phase][pvar] *
          _relative_permeability[nodenum][phase] / _fluid_viscosity[nodenum][phase];
    dm += _mass_fractions[nodenum][phase][_fluid_component] * _fluid_density_node[nodenum][phase] *
          (*_drelative_permeability_dvar)[nodenum][phase][pvar] / _fluid_viscosity[nodenum][phase];
    dm -= _mass_fractions[nodenum][phase][_fluid_component] * _fluid_density_node[nodenum][phase] *
          _relative_permeability[nodenum][phase] * (*_dfluid_viscosity_dvar)[nodenum][phase][pvar] /
          std::pow(_fluid_viscosity[nodenum][phase], 2);
    return dm;
  }
  else
    libmesh_ignore(nodenum, phase, pvar);
  return 0.0;
}

template class PorousFlowAdvectiveFluxTempl<false>;
template class PorousFlowAdvectiveFluxTempl<true>;
