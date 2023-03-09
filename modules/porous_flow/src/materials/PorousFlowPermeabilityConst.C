//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPermeabilityConst.h"

registerMooseObject("PorousFlowApp", PorousFlowPermeabilityConst);
registerMooseObject("PorousFlowApp", ADPorousFlowPermeabilityConst);

template <bool is_ad>
InputParameters
PorousFlowPermeabilityConstTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowPermeabilityBase::validParams();
  params.addRequiredParam<RealTensorValue>(
      "permeability",
      "The permeability tensor (usually in m^2), which is assumed constant for this material");
  params.addClassDescription(
      "This Material calculates the permeability tensor assuming it is constant");
  return params;
}

template <bool is_ad>
PorousFlowPermeabilityConstTempl<is_ad>::PorousFlowPermeabilityConstTempl(
    const InputParameters & parameters)
  : PorousFlowPermeabilityBaseTempl<is_ad>(parameters),
    _input_permeability(this->template getParam<RealTensorValue>("permeability"))
{
}

template <bool is_ad>
void
PorousFlowPermeabilityConstTempl<is_ad>::computeQpProperties()
{
  _permeability_qp[_qp] = _input_permeability;

  if (!is_ad)
  {
    (*_dpermeability_qp_dvar)[_qp].assign(_num_var, RealTensorValue());
    (*_dpermeability_qp_dgradvar)[_qp].resize(LIBMESH_DIM);
    for (unsigned i = 0; i < LIBMESH_DIM; ++i)
      (*_dpermeability_qp_dgradvar)[_qp][i].assign(_num_var, RealTensorValue());
  }
}

template class PorousFlowPermeabilityConstTempl<false>;
template class PorousFlowPermeabilityConstTempl<true>;
