//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
