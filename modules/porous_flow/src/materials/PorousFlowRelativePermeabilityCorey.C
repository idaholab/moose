//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowRelativePermeabilityCorey.h"

registerMooseObject("PorousFlowApp", PorousFlowRelativePermeabilityCorey);
registerMooseObject("PorousFlowApp", ADPorousFlowRelativePermeabilityCorey);

template <bool is_ad>
InputParameters
PorousFlowRelativePermeabilityCoreyTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowRelativePermeabilityBaseTempl<is_ad>::validParams();
  params.addRequiredParam<Real>("n", "The Corey exponent of the phase.");
  params.addClassDescription("This Material calculates relative permeability of the fluid phase, "
                             "using the simple Corey model ((S-S_res)/(1-sum(S_res)))^n");
  return params;
}

template <bool is_ad>
PorousFlowRelativePermeabilityCoreyTempl<is_ad>::PorousFlowRelativePermeabilityCoreyTempl(
    const InputParameters & parameters)
  : PorousFlowRelativePermeabilityBaseTempl<is_ad>(parameters),
    _n(this->template getParam<Real>("n"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PorousFlowRelativePermeabilityCoreyTempl<is_ad>::relativePermeability(GenericReal<is_ad> seff) const
{
  return std::pow(seff, _n);
}

template <bool is_ad>
Real
PorousFlowRelativePermeabilityCoreyTempl<is_ad>::dRelativePermeability(Real seff) const
{
  return _n * std::pow(seff, _n - 1.0);
}

template class PorousFlowRelativePermeabilityCoreyTempl<false>;
template class PorousFlowRelativePermeabilityCoreyTempl<true>;
