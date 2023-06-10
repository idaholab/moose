//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowRelativePermeabilityBC.h"
#include "PorousFlowBrooksCorey.h"

registerMooseObject("PorousFlowApp", PorousFlowRelativePermeabilityBC);
registerMooseObject("PorousFlowApp", ADPorousFlowRelativePermeabilityBC);

template <bool is_ad>
InputParameters
PorousFlowRelativePermeabilityBCTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowRelativePermeabilityBaseTempl<is_ad>::validParams();
  params.addRequiredParam<Real>("lambda", "The Brooks-Corey exponent of the phase");
  params.addParam<bool>("nw_phase", false, "Set true if this is the non-wetting phase");
  params.addClassDescription("Brooks-Corey relative permeability");
  return params;
}

template <bool is_ad>
PorousFlowRelativePermeabilityBCTempl<is_ad>::PorousFlowRelativePermeabilityBCTempl(
    const InputParameters & parameters)
  : PorousFlowRelativePermeabilityBaseTempl<is_ad>(parameters),
    _lambda(this->template getParam<Real>("lambda")),
    _is_nonwetting(this->template getParam<bool>("nw_phase"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PorousFlowRelativePermeabilityBCTempl<is_ad>::relativePermeability(GenericReal<is_ad> seff) const
{
  if (_is_nonwetting)
    return PorousFlowBrooksCorey::relativePermeabilityNW(seff, _lambda);
  else
    return PorousFlowBrooksCorey::relativePermeabilityW(seff, _lambda);
}

template <bool is_ad>
Real
PorousFlowRelativePermeabilityBCTempl<is_ad>::dRelativePermeability(Real seff) const
{
  if (_is_nonwetting)
    return PorousFlowBrooksCorey::dRelativePermeabilityNW(seff, _lambda);
  else
    return PorousFlowBrooksCorey::dRelativePermeabilityW(seff, _lambda);
}

template class PorousFlowRelativePermeabilityBCTempl<false>;
template class PorousFlowRelativePermeabilityBCTempl<true>;
