/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowMaterialPermeabilityConst.h"

template<>
InputParameters validParams<PorousFlowMaterialPermeabilityConst>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<RealTensorValue>("permeability", "The permeability tensor (usually in m^2), which is assumed constant for this material");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("This Material calculates the permeability tensor assuming it is constant");
  return params;
}

PorousFlowMaterialPermeabilityConst::PorousFlowMaterialPermeabilityConst(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _input_permeability(getParam<RealTensorValue>("permeability")),
    _PorousFlow_name_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),

    _permeability(declareProperty<RealTensorValue>("PorousFlow_permeability")),
    _dpermeability_dvar(declareProperty<std::vector<RealTensorValue> >("dPorousFlow_permeability_dvar"))
{
}

void
PorousFlowMaterialPermeabilityConst::initQpStatefulProperties()
{
  _permeability[_qp] = _input_permeability;
}

void
PorousFlowMaterialPermeabilityConst::computeQpProperties()
{
  _permeability[_qp] = _input_permeability;

  const unsigned int num_var = _PorousFlow_name_UO.numVariables();
  _dpermeability_dvar[_qp].resize(num_var, RealTensorValue());
}

