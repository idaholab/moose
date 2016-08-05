/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPermeabilityUnity.h"

template<>
InputParameters validParams<PorousFlowPermeabilityUnity>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("This Material calculates the permeability tensor assuming it is equal to 1 0 0  0 1 0  0 0 1");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  return params;
}

PorousFlowPermeabilityUnity::PorousFlowPermeabilityUnity(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),
    _PorousFlow_name_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),
    _num_var(_PorousFlow_name_UO.numVariables()),
    _permeability_qp(declareProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _dpermeability_qp_dvar(declareProperty<std::vector<RealTensorValue> >("dPorousFlow_permeability_qp_dvar"))
{
}

void
PorousFlowPermeabilityUnity::computeQpProperties()
{
  _dpermeability_qp_dvar[_qp].resize(_num_var, RealTensorValue());
  _permeability_qp[_qp] = RealTensorValue(1, 0, 0, 0, 1, 0, 0, 0, 1);
}
