/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowPermeabilityBase.h"

template<>
InputParameters validParams<PorousFlowPermeabilityBase>()
{
  InputParameters params = validParams<PorousFlowMaterialVectorBase>();
  params.addClassDescription("Base class for material permeability");
  return params;
}

PorousFlowPermeabilityBase::PorousFlowPermeabilityBase(const InputParameters & parameters) :
    PorousFlowMaterialVectorBase(parameters),
    _permeability_qp(declareProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _dpermeability_qp_dvar(declareProperty<std::vector<RealTensorValue> >("dPorousFlow_permeability_qp_dvar"))
{
}
