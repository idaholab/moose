//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFullySaturatedHeatAdvection.h"

registerMooseObject("PorousFlowApp", PorousFlowFullySaturatedHeatAdvection);
registerMooseObject("PorousFlowApp", ADPorousFlowFullySaturatedHeatAdvection);

template <bool is_ad>
InputParameters
PorousFlowFullySaturatedHeatAdvectionTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowFullySaturatedDarcyBaseTempl<is_ad>::validParams();
  params.addClassDescription("Heat flux that arises from the advection of a fully-saturated single "
                             "phase fluid.  No upwinding is used");
  return params;
}

template <bool is_ad>
PorousFlowFullySaturatedHeatAdvectionTempl<is_ad>::PorousFlowFullySaturatedHeatAdvectionTempl(
    const InputParameters & parameters)
  : PorousFlowFullySaturatedDarcyBaseTempl<is_ad>(parameters),
    _enthalpy(this->template getGenericMaterialProperty<std::vector<Real>, is_ad>(
        "PorousFlow_fluid_phase_enthalpy_qp")),
    _denthalpy_dvar(is_ad ? nullptr
                          : &this->template getMaterialProperty<std::vector<std::vector<Real>>>(
                                "dPorousFlow_fluid_phase_enthalpy_qp_dvar"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PorousFlowFullySaturatedHeatAdvectionTempl<is_ad>::mobility() const
{
  const unsigned ph = 0;
  return _enthalpy[_qp][ph] * PorousFlowFullySaturatedDarcyBaseTempl<is_ad>::mobility();
}

template <bool is_ad>
Real
PorousFlowFullySaturatedHeatAdvectionTempl<is_ad>::dmobility(unsigned pvar) const
{
  if constexpr (!is_ad)
  {
    const unsigned ph = 0;
    const Real darcy_mob = PorousFlowFullySaturatedDarcyBaseTempl<is_ad>::mobility();
    const Real ddarcy_mob = PorousFlowFullySaturatedDarcyBaseTempl<is_ad>::dmobility(pvar);
    return (*_denthalpy_dvar)[_qp][ph][pvar] * darcy_mob + _enthalpy[_qp][ph] * ddarcy_mob;
  }
  else
    libmesh_ignore(pvar);
  return 0.0;
}

template class PorousFlowFullySaturatedHeatAdvectionTempl<false>;
template class PorousFlowFullySaturatedHeatAdvectionTempl<true>;
