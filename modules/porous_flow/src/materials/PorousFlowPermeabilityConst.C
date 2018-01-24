/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPermeabilityConst.h"

template <>
InputParameters
validParams<PorousFlowPermeabilityConst>()
{
  InputParameters params = validParams<PorousFlowPermeabilityBase>();
  params.addRequiredParam<RealTensorValue>(
      "permeability",
      "The permeability tensor (usually in m^2), which is assumed constant for this material");
  params.addClassDescription(
      "This Material calculates the permeability tensor assuming it is constant");
  return params;
}

PorousFlowPermeabilityConst::PorousFlowPermeabilityConst(const InputParameters & parameters)
  : PorousFlowPermeabilityBase(parameters),
    _input_permeability(getParam<RealTensorValue>("permeability"))
{
}

void
PorousFlowPermeabilityConst::computeQpProperties()
{
  _permeability_qp[_qp] = _input_permeability;
  _dpermeability_qp_dvar[_qp].assign(_num_var, RealTensorValue());
  _dpermeability_qp_dgradvar[_qp].resize(LIBMESH_DIM);
  for (unsigned i = 0; i < LIBMESH_DIM; ++i)
    _dpermeability_qp_dgradvar[_qp][i].assign(_num_var, RealTensorValue());
}
