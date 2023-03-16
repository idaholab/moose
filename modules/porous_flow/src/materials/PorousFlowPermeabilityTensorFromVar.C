//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPermeabilityTensorFromVar.h"

registerMooseObject("PorousFlowApp", PorousFlowPermeabilityTensorFromVar);

InputParameters
PorousFlowPermeabilityTensorFromVar::validParams()
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

PorousFlowPermeabilityTensorFromVar::PorousFlowPermeabilityTensorFromVar(
    const InputParameters & parameters)
  : PorousFlowPermeabilityBase(parameters),
    _perm(coupledValue("perm")),
    _k_anisotropy(parameters.isParamValid("k_anisotropy")
                      ? getParam<RealTensorValue>("k_anisotropy")
                      : RealTensorValue(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0))
{
}

void
PorousFlowPermeabilityTensorFromVar::computeQpProperties()
{
  _permeability_qp[_qp] = _k_anisotropy * _perm[_qp];

  (*_dpermeability_qp_dvar)[_qp].resize(_num_var, RealTensorValue());
  (*_dpermeability_qp_dgradvar)[_qp].resize(LIBMESH_DIM);

  for (const auto i : make_range(Moose::dim))
    (*_dpermeability_qp_dgradvar)[_qp][i].resize(_num_var, RealTensorValue());
}
