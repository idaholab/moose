/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPermeabilityConst.h"

template<>
InputParameters validParams<PorousFlowPermeabilityConst>()
{
  InputParameters params = validParams<PorousFlowMaterialVectorBase>();
  params.addRequiredParam<RealTensorValue>("permeability", "The permeability tensor (usually in m^2), which is assumed constant for this material");
  params.addClassDescription("This Material calculates the permeability tensor assuming it is constant");
  return params;
}

PorousFlowPermeabilityConst::PorousFlowPermeabilityConst(const InputParameters & parameters) :
    PorousFlowMaterialVectorBase(parameters),
    _input_permeability(getParam<RealTensorValue>("permeability")),
    _permeability(declareProperty<RealTensorValue>("PorousFlow_permeability")),
    _dpermeability_dvar(declareProperty<std::vector<RealTensorValue> >("dPorousFlow_permeability_dvar"))
{
}

void
PorousFlowPermeabilityConst::initQpStatefulProperties()
{
  _permeability[_qp] = _input_permeability;
}

void
PorousFlowPermeabilityConst::computeQpProperties()
{
  _permeability[_qp] = _input_permeability;
  _dpermeability_dvar[_qp].resize(_num_var, RealTensorValue());
}
