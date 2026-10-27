//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFullySaturatedUpwindHeatAdvection.h"

registerMooseObject("PorousFlowApp", PorousFlowFullySaturatedUpwindHeatAdvection);
registerMooseObject("PorousFlowApp", ADPorousFlowFullySaturatedUpwindHeatAdvection);

template <bool is_ad>
InputParameters
PorousFlowFullySaturatedUpwindHeatAdvectionTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowDarcyBaseTempl<is_ad>::validParams();
  params.addClassDescription("Heat advection by a fluid.  The fluid is assumed to have a single "
                             "phase, and the advection is fully upwinded");
  return params;
}

template <bool is_ad>
PorousFlowFullySaturatedUpwindHeatAdvectionTempl<
    is_ad>::PorousFlowFullySaturatedUpwindHeatAdvectionTempl(const InputParameters & parameters)
  : PorousFlowDarcyBaseTempl<is_ad>(parameters),
    _enthalpy(this->template getGenericMaterialProperty<std::vector<Real>, is_ad>(
        "PorousFlow_fluid_phase_enthalpy_nodal")),
    _denthalpy_dvar(is_ad ? nullptr
                          : &this->template getMaterialProperty<std::vector<std::vector<Real>>>(
                                "dPorousFlow_fluid_phase_enthalpy_nodal_dvar"))
{
  if (_dictator.numPhases() != 1)
    mooseError("PorousFlowFullySaturatedUpwindHeatAdvection should not be used for multi-phase "
               "scenarios as it does not include relative-permeability effects");
}

template <bool is_ad>
GenericReal<is_ad>
PorousFlowFullySaturatedUpwindHeatAdvectionTempl<is_ad>::mobility(unsigned nodenum,
                                                                  unsigned phase) const
{
  return _enthalpy[nodenum][phase] * _fluid_density_node[nodenum][phase] /
         _fluid_viscosity[nodenum][phase];
}

template <bool is_ad>
Real
PorousFlowFullySaturatedUpwindHeatAdvectionTempl<is_ad>::dmobility(unsigned nodenum,
                                                                   unsigned phase,
                                                                   unsigned pvar) const
{
  if constexpr (!is_ad)
  {
    Real dm = (*_denthalpy_dvar)[nodenum][phase][pvar] * _fluid_density_node[nodenum][phase] /
              _fluid_viscosity[nodenum][phase];
    dm += _enthalpy[nodenum][phase] * (*_dfluid_density_node_dvar)[nodenum][phase][pvar] /
          _fluid_viscosity[nodenum][phase];
    dm -= _enthalpy[nodenum][phase] * _fluid_density_node[nodenum][phase] *
          (*_dfluid_viscosity_dvar)[nodenum][phase][pvar] /
          Utility::pow<2>(_fluid_viscosity[nodenum][phase]);
    return dm;
  }
  else
    libmesh_ignore(nodenum, phase, pvar);
  return 0.0;
}

template class PorousFlowFullySaturatedUpwindHeatAdvectionTempl<false>;
template class PorousFlowFullySaturatedUpwindHeatAdvectionTempl<true>;
