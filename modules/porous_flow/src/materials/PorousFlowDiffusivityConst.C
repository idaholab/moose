//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowDiffusivityConst.h"

registerMooseObject("PorousFlowApp", PorousFlowDiffusivityConst);
registerMooseObject("PorousFlowApp", ADPorousFlowDiffusivityConst);

template <bool is_ad>
InputParameters
PorousFlowDiffusivityConstTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowDiffusivityBaseTempl<is_ad>::validParams();
  params.addRequiredParam<std::vector<Real>>(
      "tortuosity", "List of tortuosities. Order is i) phase 0; ii) phase 1; etc");
  params.addClassDescription(
      "This Material provides constant tortuosity and diffusion coefficients");
  return params;
}

template <bool is_ad>
PorousFlowDiffusivityConstTempl<is_ad>::PorousFlowDiffusivityConstTempl(
    const InputParameters & parameters)
  : PorousFlowDiffusivityBaseTempl<is_ad>(parameters),
    _input_tortuosity(this->template getParam<std::vector<Real>>("tortuosity"))
{
  // Check that the number of tortuosities entered is equal to the number of phases
  if (_input_tortuosity.size() != _num_phases)
    this->template paramError(
        "tortuosity",
        "The number of tortuosity values entered is not equal to the number of phases "
        "specified in the Dictator");

  // Check that all tortuosities are (0, 1]
  for (unsigned int i = 0; i < _num_phases; ++i)
    if (_input_tortuosity[i] <= 0.0 || _input_tortuosity[i] > 1)
      this->template paramError(
          "tortuosity",
          "All tortuosities must be greater than zero and less than (or equal to) one"
          ".\nNote: the definition of tortuosity used is l/le, where l is the straight line "
          "length and le is the effective flow length");
}

template <bool is_ad>
void
PorousFlowDiffusivityConstTempl<is_ad>::computeQpProperties()
{
  PorousFlowDiffusivityBaseTempl<is_ad>::computeQpProperties();

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
    _tortuosity[_qp][ph] = _input_tortuosity[ph];
}

template class PorousFlowDiffusivityConstTempl<false>;
template class PorousFlowDiffusivityConstTempl<true>;
