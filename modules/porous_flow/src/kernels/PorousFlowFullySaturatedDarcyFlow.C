//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFullySaturatedDarcyFlow.h"

registerMooseObject("PorousFlowApp", PorousFlowFullySaturatedDarcyFlow);
registerMooseObject("PorousFlowApp", ADPorousFlowFullySaturatedDarcyFlow);

template <bool is_ad>
InputParameters
PorousFlowFullySaturatedDarcyFlowTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowFullySaturatedDarcyBaseTempl<is_ad>::validParams();
  params.addParam<unsigned int>(
      "fluid_component", 0, "The index corresponding to the fluid component for this kernel");
  params.addClassDescription(
      "Darcy flux suitable for models involving a fully-saturated single phase, multi-component "
      "fluid.  No upwinding is used.  Templated on is_ad: the AD version requires no hand-coded "
      "Jacobian.");
  return params;
}

template <bool is_ad>
PorousFlowFullySaturatedDarcyFlowTempl<is_ad>::PorousFlowFullySaturatedDarcyFlowTempl(
    const InputParameters & parameters)
  : PorousFlowFullySaturatedDarcyBaseTempl<is_ad>(parameters),
    _mfrac(this->template getGenericMaterialProperty<std::vector<std::vector<Real>>, is_ad>(
        "PorousFlow_mass_frac_qp")),
    _dmfrac_dvar(
        is_ad ? nullptr
              : &this->template getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
                    "dPorousFlow_mass_frac_qp_dvar")),
    _fluid_component(this->template getParam<unsigned int>("fluid_component"))
{
  if (_fluid_component >= _dictator.numComponents())
    this->paramError(
        "fluid_component",
        "The Dictator proclaims that the maximum fluid component index in this simulation is ",
        _dictator.numComponents() - 1,
        " whereas you have used ",
        _fluid_component,
        ". Remember that indexing starts at 0. Happiness equals perfection.");
}

template <bool is_ad>
GenericReal<is_ad>
PorousFlowFullySaturatedDarcyFlowTempl<is_ad>::mobility() const
{
  const unsigned ph = 0;
  return _mfrac[_qp][ph][_fluid_component] *
         PorousFlowFullySaturatedDarcyBaseTempl<is_ad>::mobility();
}

template <bool is_ad>
Real
PorousFlowFullySaturatedDarcyFlowTempl<is_ad>::dmobility(unsigned int pvar) const
{
  if constexpr (!is_ad)
  {
    const unsigned ph = 0;
    const Real darcy_mob = PorousFlowFullySaturatedDarcyBaseTempl<is_ad>::mobility();
    const Real ddarcy_mob = PorousFlowFullySaturatedDarcyBaseTempl<is_ad>::dmobility(pvar);
    return (*_dmfrac_dvar)[_qp][ph][_fluid_component][pvar] * darcy_mob +
           _mfrac[_qp][ph][_fluid_component] * ddarcy_mob;
  }
  else
    libmesh_ignore(pvar);
  return 0.0;
}

template class PorousFlowFullySaturatedDarcyFlowTempl<false>;
template class PorousFlowFullySaturatedDarcyFlowTempl<true>;
