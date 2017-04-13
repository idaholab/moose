/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPermeabilityBase.h"

template <>
InputParameters
validParams<PorousFlowPermeabilityBase>()
{
  InputParameters params = validParams<PorousFlowMaterialVectorBase>();
  params.addClassDescription("Base class for material permeability");
  params.set<bool>("at_nodes") = false;
  return params;
}

PorousFlowPermeabilityBase::PorousFlowPermeabilityBase(const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _permeability_qp(declareProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _dpermeability_qp_dvar(
        declareProperty<std::vector<RealTensorValue>>("dPorousFlow_permeability_qp_dvar")),
    _dpermeability_qp_dgradvar(declareProperty<std::vector<std::vector<RealTensorValue>>>(
        "dPorousFlow_permeability_qp_dgradvar"))
{
  if (_nodal_material == true)
    mooseError("PorousFlowPermeability classes are only defined for at_nodes = false");
}
