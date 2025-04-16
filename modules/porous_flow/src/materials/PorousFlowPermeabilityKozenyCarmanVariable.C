//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPermeabilityKozenyCarmanVariable.h"

registerMooseObject("PorousFlowApp", PorousFlowPermeabilityKozenyCarmanVariable);
registerMooseObject("PorousFlowApp", ADPorousFlowPermeabilityKozenyCarmanVariable);

template <bool is_ad>
InputParameters
PorousFlowPermeabilityKozenyCarmanVariableTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowPermeabilityKozenyCarmanBase::validParams();
  params.addRequiredCoupledVar("A", "Variable used in permeability function.");
  params.addClassDescription("This Material calculates the permeability tensor from the "
                             "Kozeny-Carman equation for spatially varying initial properties.");
  return params;
}

template <bool is_ad>
PorousFlowPermeabilityKozenyCarmanVariableTempl<
    is_ad>::PorousFlowPermeabilityKozenyCarmanVariableTempl(const InputParameters & parameters)
  : PorousFlowPermeabilityKozenyCarmanBaseTempl<is_ad>(parameters), _A(coupledValue("A"))
{
}

template <bool is_ad>
Real
PorousFlowPermeabilityKozenyCarmanVariableTempl<is_ad>::computeA() const
{
  return _A[_qp];
}

template class PorousFlowPermeabilityKozenyCarmanVariableTempl<false>;
template class PorousFlowPermeabilityKozenyCarmanVariableTempl<true>;
