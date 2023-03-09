//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowRelativePermeabilityConst.h"

registerMooseObject("PorousFlowApp", PorousFlowRelativePermeabilityConst);
registerMooseObject("PorousFlowApp", ADPorousFlowRelativePermeabilityConst);

template <bool is_ad>
InputParameters
PorousFlowRelativePermeabilityConstTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowRelativePermeabilityBaseTempl<is_ad>::validParams();
  params.addParam<Real>("kr", 1.0, "Relative permeability");
  params.addClassDescription(
      "This class sets the relative permeability to a constant value (default = 1)");
  return params;
}

template <bool is_ad>
PorousFlowRelativePermeabilityConstTempl<is_ad>::PorousFlowRelativePermeabilityConstTempl(
    const InputParameters & parameters)
  : PorousFlowRelativePermeabilityBaseTempl<is_ad>(parameters),
    _relperm(this->template getParam<Real>("kr"))
{
}

template <bool is_ad>
GenericReal<is_ad> PorousFlowRelativePermeabilityConstTempl<is_ad>::relativePermeability(
    GenericReal<is_ad> /*seff*/) const
{
  return _relperm;
}

template <bool is_ad>
Real PorousFlowRelativePermeabilityConstTempl<is_ad>::dRelativePermeability(Real /*seff*/) const
{
  return 0.0;
}

template class PorousFlowRelativePermeabilityConstTempl<false>;
template class PorousFlowRelativePermeabilityConstTempl<true>;
