//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPermeabilityTensorFromVar.h"

registerMooseObject("PorousFlowApp", PorousFlowPermeabilityTensorFromVar);
registerMooseObject("PorousFlowApp", ADPorousFlowPermeabilityTensorFromVar);

template <bool is_ad>
InputParameters
PorousFlowPermeabilityTensorFromVarTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowPermeabilityBase::validParams();
  params.addRequiredCoupledVar("perm", "The scalar permeability");
  params.addParam<RealTensorValue>("k_anisotropy",
                                   "A tensor to multiply the scalar "
                                   "permeability, in order to obtain anisotropy if "
                                   "required. Defaults to isotropic permeability "
                                   "if not specified.");
  params.addClassDescription(
      "This Material calculates the permeability tensor from a coupled variable "
      "multiplied by a tensor");
  return params;
}

template <bool is_ad>
PorousFlowPermeabilityTensorFromVarTempl<is_ad>::PorousFlowPermeabilityTensorFromVarTempl(
    const InputParameters & parameters)
  : PorousFlowPermeabilityBaseTempl<is_ad>(parameters),
    _perm(coupledValue("perm")),
    _k_anisotropy(parameters.isParamValid("k_anisotropy")
                      ? this->template getParam<RealTensorValue>("k_anisotropy")
                      : RealTensorValue(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0))
{
}

template <bool is_ad>
void
PorousFlowPermeabilityTensorFromVarTempl<is_ad>::computeQpProperties()
{
  _permeability_qp[_qp] = _k_anisotropy * _perm[_qp];

  if (!is_ad)
  {
    (*_dpermeability_qp_dvar)[_qp].resize(_num_var, RealTensorValue());
    (*_dpermeability_qp_dgradvar)[_qp].resize(LIBMESH_DIM);

    for (const auto i : make_range(Moose::dim))
      (*_dpermeability_qp_dgradvar)[_qp][i].resize(_num_var, RealTensorValue());
  }
}

template class PorousFlowPermeabilityTensorFromVarTempl<false>;
template class PorousFlowPermeabilityTensorFromVarTempl<true>;
